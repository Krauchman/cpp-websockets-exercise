#pragma once

#include <coinbase/subscribe.h>
#include <coinbase/message.h>
#include <websocket/websocket.h>
#include <shared_queue/queue_abstract.h>

#include <boost/asio.hpp>

#include <vector>
#include <string>

namespace coinbase::ticker_logger {
    using ticker_message = message::ticker_message;
    using ticker_subscriber = subscribe::ticker_subscriber;

    struct config {
        std::string output_file;
        std::vector<std::string> product_ids;
        bool use_lock_queue = false;
    };

    class got_termination_message : public std::exception {};

    class runner {
    public:
        runner(boost::asio::io_context& ioc, config conf);
        runner(std::unique_ptr<websocket::session_base> ws_session, config conf);
        runner(std::unique_ptr<ticker_subscriber> subscriber, config conf);

        void start();
    private:
        void start_logger_thread();
        void subscribe();
        void wait_logger_thread();

        config conf_;
        std::shared_ptr<shared_queue::shared_queue_base<ticker_message>> queue_;
        std::unique_ptr<ticker_subscriber> subscriber_;
        std::unique_ptr<logging::csv_logger> logger_;
        std::future<void> logger_thread_ft_;
    };
} // coinbase::ticker_logger
