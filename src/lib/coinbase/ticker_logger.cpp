#include "ticker_logger.h"

#include <logging/logging.h>

#include <shared_queue/consumer.h>
#include <shared_queue/lock_queue.h>
#include <shared_queue/spsc_lockfree_queue.h>

#include <boost/log/trivial.hpp>

#include <thread>

namespace coinbase::ticker_logger {

    runner::runner(boost::asio::io_context& ioc, config conf)
        : runner(std::move(std::make_unique<ticker_subscriber>(ioc, conf.product_ids)), conf)
    {
    }

    runner::runner(std::unique_ptr<websocket::session_base> ws_session, config conf)
        : runner(std::move(std::make_unique<ticker_subscriber>(std::move(ws_session), conf.product_ids)), conf)
    {
    }

    runner::runner(std::unique_ptr<ticker_subscriber> subscriber, config conf)
        : conf_(std::move(conf))
        , subscriber_(std::move(subscriber))
        , logger_(std::make_unique<logging::csv_logger>(conf_.output_file))
    {
        BOOST_LOG_TRIVIAL(info) << "Output path: " << conf_.output_file;
        BOOST_LOG_TRIVIAL(info) << "Number of product IDs to subscribe to: " << conf_.product_ids.size();

        if (conf_.use_lock_queue) {
            BOOST_LOG_TRIVIAL(info) << "Thead-shared queue implementation: locking queue";
            queue_ = std::make_shared<shared_queue::lock_queue<ticker_message>>();
        } else {
            BOOST_LOG_TRIVIAL(info) << "Thead-shared queue implementation: lock-free queue";
            queue_ = std::make_shared<shared_queue::spsc_lockfree_queue<ticker_message>>(500);
        }
    }

    void runner::start() {
        BOOST_LOG_TRIVIAL(info) << "Runner starting";

        start_logger_thread();
        subscribe();
        wait_logger_thread();

        BOOST_LOG_TRIVIAL(info) << "Done";
    }

    void runner::start_logger_thread() {
        logger_->log_header<ticker_message>();

        shared_queue::looped_consumer<ticker_message> consumer(queue_);
        
        logger_thread_ft_ = std::async(std::launch::async, consumer, [this](const ticker_message& message) {
            if (message.is_terminational()) {
                throw got_termination_message();
            }

            logger_->log(message);
        });
        BOOST_LOG_TRIVIAL(info) << "Started the logger thread";
    }

    void runner::subscribe() {
        try {
            BOOST_LOG_TRIVIAL(info) << "Starting Coinbase ticker channel subscriber";
            subscriber_->start([this](const std::string& message) {

                if (!queue_->push({message})) {
                    BOOST_LOG_TRIVIAL(error) << "Failed to push message to the queue: allocation is full";
                }
            });
        } catch (std::exception e) {
            BOOST_LOG_TRIVIAL(error) << "Subscriber thread caught exception, stopping. Exception: " << e.what();
            ticker_message termination_message;
            termination_message.make_terminational();
            queue_->push(termination_message);
        }
    }

    void runner::wait_logger_thread() {
        BOOST_LOG_TRIVIAL(info) << "Waiting for the logger thread to finish";
        logger_thread_ft_.wait();
    }

} // coinbase::ticker_logger
