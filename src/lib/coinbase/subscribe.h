#pragma once

#include <websocket/websocket.h>

#include <boost/asio.hpp>

namespace coinbase::subscribe {
    namespace net = boost::asio;

    class subscriber_base {
    public:
        virtual ~subscriber_base() = default;

        virtual void start(std::function<void(const std::string&)> callback) = 0;
    };

    class ticker_subscriber : public subscriber_base {
    public:
        ticker_subscriber(net::io_context& ioc, std::vector<std::string> product_ids);
        ticker_subscriber(std::unique_ptr<websocket::session_base> session, std::vector<std::string> product_ids);

        void start(std::function<void(const std::string&)> callback) override;
    
    private:
        bool subscribe();

        std::unique_ptr<websocket::session_base> session_;
        std::vector<std::string> product_ids_;
    };

} // coinbase::subscribe
