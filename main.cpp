#include "websocket.h"
#include "logging/logging.h"
#include "coinbase/message.h"

#include <iostream>
#include <exception>
#include <thread>
#include <chrono>

#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <boost/json.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/log/trivial.hpp>

namespace net = boost::asio;
namespace beast = boost::beast;
namespace json = boost::json;
namespace lockfree = boost::lockfree;

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

        websocket::session session_;
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
    auto q = std::make_shared<lockfree::spsc_queue<coinbase::message::ticker_message>>(200);
    
    // logger thread
    logging::csv_logger logger("output.csv");
    queue::looped_consumer consumer(q);
    auto ft = std::async(std::launch::async, consumer, [&logger](const coinbase::message::ticker_message& message) {
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
