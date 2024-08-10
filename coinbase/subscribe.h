#pragma once

#include "../websocket.h"

#include <boost/beast.hpp>
#include <boost/asio.hpp>

namespace coinbase::subscribe {
    namespace net = boost::asio;
    namespace beast = boost::beast;

    class subscriber_base {
    public:
        virtual void start(std::function<void(const std::string&)> callback) = 0;
    };

    class ticker_subscriber : public subscriber_base {
    public:
        ticker_subscriber(net::io_context& ioc);

        void start(std::function<void(const std::string&)> callback) override;
    
    private:
        void subscribe();

        websocket::session session_;
        beast::flat_buffer buffer_;
    };

}; // coinbase::subscribe
