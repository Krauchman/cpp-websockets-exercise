#pragma once

#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/asio.hpp>

namespace websocket {
    namespace net = boost::asio;
    namespace beast = boost::beast;
    namespace ssl = boost::asio::ssl;
    using tcp = net::ip::tcp;

    class session_base {
    public:
        virtual ~session_base() = default;

        virtual void start() = 0;
        virtual void write(const std::string& message) = 0;
        virtual std::string read() = 0;
        virtual void close() = 0;
    };
    
    class session : public session_base {
    public:
        session(std::string host, std::string port, net::io_context& ioc);

        void start() override;

        void write(const std::string& message) override;

        std::string read() override;

        void close() override;

    private:
        tcp::resolver resolver_;
        ssl::context ctx_;
        beast::websocket::stream<beast::ssl_stream<net::ip::tcp::socket>> ws_;
        beast::flat_buffer buffer_;
        std::string host_;
        std::string port_;
        bool started_ = false;
    };

} // websocket
