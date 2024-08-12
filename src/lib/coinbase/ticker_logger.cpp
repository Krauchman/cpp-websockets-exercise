#include "ticker_logger.h"

#include <coinbase/message.h>
#include <logging/logging.h>

#include <shared_queue/consumer.h>
#include <shared_queue/lock_queue.h>
#include <shared_queue/spsc_lockfree_queue.h>

#include <boost/log/trivial.hpp>

#include <thread>

namespace coinbase::ticker_logger {
    using ticker_message = message::ticker_message;

    runner::runner(boost::asio::io_context& ioc, config conf)
        : conf_(std::move(conf))
        , subscriber_(ioc, conf_.product_ids)
    {
    }

    runner::runner(std::unique_ptr<websocket::session_base> ws_session, config conf)
        : conf_(std::move(conf))
        , subscriber_(std::move(ws_session), conf_.product_ids)
    {
    }

    void runner::start() {
        BOOST_LOG_TRIVIAL(info) << "Output path: " << conf_.output_file;
        BOOST_LOG_TRIVIAL(info) << "Number of product IDs to subscribe to: " << conf_.product_ids.size();

        // initializing a shared queue
        std::shared_ptr<shared_queue::shared_queue_base<ticker_message>> queue_ptr;
        if (conf_.use_lock_queue) {
            BOOST_LOG_TRIVIAL(info) << "Thead-shared queue implementation: locking queue";
            queue_ptr = std::make_shared<shared_queue::lock_queue<ticker_message>>();
        } else {
            BOOST_LOG_TRIVIAL(info) << "Thead-shared queue implementation: lock-free queue";
            queue_ptr = std::make_shared<shared_queue::spsc_lockfree_queue<ticker_message>>(100);
        }
        
        // starting the logger thread
        logging::csv_logger logger(conf_.output_file);
        logger.log_header<ticker_message>();
        shared_queue::looped_consumer<ticker_message> consumer(queue_ptr);
        
        auto logger_thread_ft = std::async(std::launch::async, consumer, [&logger](const ticker_message& message) {
            if (message.is_terminational()) {
                throw got_termination_message();
            }

            logger.log(message);
        });
        BOOST_LOG_TRIVIAL(info) << "Started the logger thread";

        // the main thread logic (processing websockets and parsing JSONs)
        try {
            BOOST_LOG_TRIVIAL(info) << "Starting Coinbase ticker channel subscriber";
            subscriber_.start([queue_ptr](const std::string& message) {
                if (!queue_ptr->push({message})) {
                    BOOST_LOG_TRIVIAL(error) << "Failed to push message to the queue: allocation is full";
                }
            });
        } catch (std::exception e) {
            BOOST_LOG_TRIVIAL(error) << "Subscriber thread caught exception, stopping. Exception: " << e.what();
            ticker_message termination_message;
            termination_message.make_terminational();
            queue_ptr->push(termination_message);
        }

        BOOST_LOG_TRIVIAL(info) << "Waiting for the logger thread to finish";
        logger_thread_ft.wait();

        BOOST_LOG_TRIVIAL(info) << "Done";
    }

} // coinbase::ticker_logger
