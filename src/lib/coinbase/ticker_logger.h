#pragma once

#include <coinbase/subscribe.h>
#include <websocket/websocket.h>

#include <boost/asio.hpp>

#include <vector>
#include <string>

namespace coinbase::ticker_logger {
    struct config {
        std::string output_file;
        std::vector<std::string> product_ids;
        bool use_lock_queue = false;
    };

    class runner {
    public:
        runner(boost::asio::io_context& ioc, config conf);
        runner(std::unique_ptr<websocket::session_base> ws_session, config conf);

        void start();
    private:
        config conf_;
        coinbase::subscribe::ticker_subscriber subscriber_;
    };
} // coinbase::ticker_logger
