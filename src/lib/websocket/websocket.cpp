#include "websocket.h"

#include <exception>
#include <chrono>

namespace websocket {
    namespace http = beast::http;
    
    session::session(std::string host, std::string port, net::io_context& ioc)
        : resolver_(ioc)
        , ctx_(ssl::context::tlsv12_client)
        , ws_(ioc, ctx_)
        , host_(std::move(host))
        , port_(std::move(port))
    {
    }

    void session::start() {
        auto ep = net::connect(get_lowest_layer(ws_), resolver_.resolve(host_, port_));

        // Set SNI Hostname
        if (!SSL_set_tlsext_host_name(ws_.next_layer().native_handle(), host_.c_str())) {
            throw std::runtime_error("Failed to set SNI Hostname");
        }

        // SSL handshake
        ws_.next_layer().handshake(ssl::stream_base::client);

        ws_.set_option(beast::websocket::stream_base::decorator(
            [](beast::websocket::request_type& req)
            {
                req.set(http::field::user_agent,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                        " websocket-client-coro");
            }));

        // Websocket handshake
        ws_.handshake(host_ + ":" + std::to_string(ep.port()), "/");
        started_ = true;
    }

    void session::write(const std::string& message) {
        if (!started_) {
            throw std::logic_error("Failed to send message: session has not started yet");
        }
        
        ws_.write(net::buffer(message));
    }

    std::string session::read() {
        if (!started_) {
            throw std::logic_error("Failed to read message: session has not started yet");
        }

        buffer_.consume(buffer_.size());
        ws_.read(buffer_);
        return beast::buffers_to_string(buffer_.data());
    }

    void session::close() {
        if (!started_) {
            throw std::logic_error("Failed to close connection: session has not started yet");
        }

        ws_.close(beast::websocket::close_code::normal);
    }

} // websocket
