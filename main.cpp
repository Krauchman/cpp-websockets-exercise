#include <iostream>
#include <exception>
#include <thread>
#include <chrono>
#include <ostream>
#include <ratio>

#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <boost/json.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/log/trivial.hpp>

namespace net = boost::asio;
namespace beast = boost::beast;
namespace ssl = boost::asio::ssl;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace json = boost::json;
namespace lockfree = boost::lockfree;
namespace io = boost::iostreams;
using tcp = net::ip::tcp;

namespace session {
    class session {
    public:
        explicit session(std::string host, std::string port, net::io_context& ioc)
            : resolver_(ioc)
            , ctx_(ssl::context::tlsv12_client)
            , ws_(ioc, ctx_)
            , host_(host)
            , port_(port)
        {
        }

        void start() {
            auto ep = net::connect(get_lowest_layer(ws_), resolver_.resolve(host_, port_));

            // Set SNI Hostname
            if (!SSL_set_tlsext_host_name(ws_.next_layer().native_handle(), host_.c_str())) {
                throw "Failed to set SNI Hostname";
            }

            // SSL handshake
            ws_.next_layer().handshake(ssl::stream_base::client);

            ws_.set_option(websocket::stream_base::decorator(
                [](websocket::request_type& req)
                {
                    req.set(http::field::user_agent,
                        std::string(BOOST_BEAST_VERSION_STRING) +
                            " websocket-client-coro");
                }));

            // Websocket handshake
            ws_.handshake(host_ + ":" + std::to_string(ep.port()), "/");
            started_ = true;
        }

        void write(std::string& message) {
            if (!started_) {
                throw "Failed to send message: session has not started yet";
            }
            
            ws_.write(net::buffer(message));
        }

        void read(beast::flat_buffer& buffer) {
            if (!started_) {
                throw "Failed to read message: session has not started yet";
            }
            // auto start_ts = std::chrono::high_resolution_clock::now();
            ws_.read(buffer);
            // const auto dur = std::chrono::high_resolution_clock::now() - start_ts;
            // std::cout << std::chrono::nanoseconds(dur).count() << std::endl;
        }

        void close() {
            if (!started_) {
                throw "Failed to close connection: session has not started yet";
            }

            ws_.close(websocket::close_code::normal);
        }

    private:
        tcp::resolver resolver_;
        ssl::context ctx_;
        websocket::stream<beast::ssl_stream<net::ip::tcp::socket>> ws_;
        std::string host_;
        std::string port_;
        bool started_ = false;
    };
} // session

namespace logging {
    
    template <typename T>
    class logger_base {
    public:
        virtual void log(const T& entry) = 0;
    };

    template <typename T>
    class file_logger_base : public logger_base<T> {
    public:
        file_logger_base(std::string path)
            : buffer_(path)
            , out_(&buffer_)
        {
        }

        void log(const T& entry) override {
            out_ << get_log_repr(entry) << std::endl;
        }

    protected:
        virtual std::string get_log_repr(const T& entry) = 0;

        io::stream_buffer<io::file_sink> buffer_;
        std::ostream out_;
    };

    class csv_row_convertible {
    public:
        virtual std::string to_csv_row() const = 0;
    };

    class csv_logger : public file_logger_base<csv_row_convertible> {
    public:
        csv_logger(std::string path)
            : file_logger_base<csv_row_convertible>(path)
        {
        }
    
    protected:
        std::string get_log_repr(const csv_row_convertible& entry) override {
            return entry.to_csv_row();
        }
    };

} // logging

namespace coinbase {

    class message_base : public logging::csv_row_convertible {
    public:
        message_base() {
        }
        message_base(std::string message_str)
            : data_(std::move(json::parse(message_str)))
        {
        }

        std::string to_csv_row() const override {
            std::string result;
            for (auto field_name : get_field_names()) {
                auto value_ptr = data_.as_object().if_contains(field_name);

                if (value_ptr) {
                    result += json::serialize(*value_ptr);
                }
                result.push_back(',');
            }
            return result;
        }

        virtual std::vector<std::string> get_field_names() const = 0;

    private:
        json::value data_;
    };

    class ticker_message : public message_base {
    public:
        ticker_message() : message_base() {}
        ticker_message(std::string message_str) 
            : message_base(message_str)
        {
        }

        std::vector<std::string> get_field_names() const override {
            return {"type", "sequence", "product_id", "price",
                    "open_24h", "volume_24h", "low_24h",
                    "high_24h", "volume_30d", "best_bid",
                    "best_bid_size", "best_ask", "best_ask_size",
                    "side", "time", "trade_id", "last_size"};
        };
    };
} // coinbase

namespace subscribe {

    class subscriber_base {
    public:
        virtual void start(std::function<void(const std::string&)> callback) = 0;
    };

    class coinbase_ticker_subscriber : public subscriber_base {
    public:
        coinbase_ticker_subscriber(net::io_context& ioc)
            : session_("ws-feed.exchange.coinbase.com", "443", ioc)
        {
        }

        void start(std::function<void(const std::string&)> callback) override {
            session_.start();

            subscribe();

            while (true) {
                session_.read(buffer_);
                // q->push({beast::buffers_to_string(buffer.data())});
                callback(beast::buffers_to_string(buffer_.data()));
                buffer_.consume(buffer_.size());
            }
        }
    
    private:
        void subscribe() {
            json::value subscribe_message_data = {
                { "type", "subscribe" },
                { "product_ids", json::array{"ETH-USD", "BTC-USD"} },
                { "channels", json::array{"ticker"} }
            };
            std::string subscribe_message = json::serialize(subscribe_message_data);
            
            session_.write(subscribe_message);

            session_.read(buffer_);
            // std::cout << beast::make_printable(buffer_.data()) << std::endl;
            BOOST_LOG_TRIVIAL(info) << "Subscribe response from the exchange: " << beast::make_printable(buffer_.data());
            buffer_.consume(buffer_.size());
        }

        session::session session_;
        beast::flat_buffer buffer_;
    };
}; // subscribe

namespace queue {
    
    template <typename T>
    class looped_consumer {
    public:
        looped_consumer(std::shared_ptr<lockfree::spsc_queue<T>> q_ptr)
            : q_ptr_(q_ptr)
        {
        }

        void operator()(std::function<void(const T&)> callback) {
            while (true) {
                T message;
                while (!q_ptr_->pop(message));

                callback(message);
            }
        }
    private:
        std::shared_ptr<lockfree::spsc_queue<T>> q_ptr_;
    };

} // queue

int main() {
    // shared queue
    auto q = std::make_shared<lockfree::spsc_queue<coinbase::ticker_message>>(200);
    
    // logger thread
    logging::csv_logger logger("output.csv");
    queue::looped_consumer consumer(q);
    auto ft = std::async(std::launch::async, consumer, [&logger](const coinbase::ticker_message& message) {
        logger.log(message);
    });

    // main thread processing websockets and parsing JSONs
    net::io_context ioc;
    subscribe::coinbase_ticker_subscriber subscriber(ioc);
    subscriber.start([q](const std::string& message) {
        if (!q->push({message})) {
            BOOST_LOG_TRIVIAL(error) << "Failed to push message to the queue: allocation is full";
        }
    });

    ft.wait();

    return 0;
}
