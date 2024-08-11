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
    
    class session {
    public:
        explicit session(std::string host, std::string port, net::io_context& ioc);

        void start();

        void write(const std::string& message);

        void read(beast::flat_buffer& buffer);

        void close();

    private:
        tcp::resolver resolver_;
        ssl::context ctx_;
        beast::websocket::stream<beast::ssl_stream<net::ip::tcp::socket>> ws_;
        std::string host_;
        std::string port_;
        bool started_ = false;
    };

} // websocket
