#include "config.h"

#include "coinbase/subscribe.h"
#include "coinbase/message.h"
#include "logging/logging.h"

#include "shared_queue/consumer.h"
#include "shared_queue/lock_queue.h"
#include "shared_queue/spsc_lockfree_queue.h"

#include <boost/log/trivial.hpp>
#include <boost/asio.hpp>

#include <thread>
#include <iostream>
#include <exception>

int main(int ac, char* av[]) {
    using ticker_message = coinbase::message::ticker_message;

    ticker_logger_config config;
    parse_config(ac, av, config);
    
    BOOST_LOG_TRIVIAL(info) << "Output path: " << config.output_file;
    BOOST_LOG_TRIVIAL(info) << "Number of product IDs to subscribe to: " << config.product_ids.size();

    // initializing a shared queue
    std::shared_ptr<shared_queue::shared_queue_base<ticker_message>> q;
    if (config.use_lock_queue) {
        BOOST_LOG_TRIVIAL(info) << "Thead-shared queue implementation: locking queue";
        q = std::make_shared<shared_queue::lock_queue<ticker_message>>();
    } else {
        BOOST_LOG_TRIVIAL(info) << "Thead-shared queue implementation: lock-free queue";
        q = std::make_shared<shared_queue::spsc_lockfree_queue<ticker_message>>(200);
    }
    
    // starting the logger thread
    logging::csv_logger logger(config.output_file);
    logger.log_header<ticker_message>();
    shared_queue::looped_consumer<ticker_message> consumer(q);
    
    auto ft = std::async(std::launch::async, consumer, [&logger](const ticker_message& message) {
        logger.log(message);
    });
    BOOST_LOG_TRIVIAL(info) << "Started the logger thread";

    // the main thread logic (processing websockets and parsing JSONs)
    boost::asio::io_context ioc;
    coinbase::subscribe::ticker_subscriber subscriber(ioc, config.product_ids);
    
    BOOST_LOG_TRIVIAL(info) << "Starting Coinbase ticker channel subscriber";
    subscriber.start([q](const std::string& message) {
        if (!q->push({message})) {
            BOOST_LOG_TRIVIAL(error) << "Failed to push message to the queue: allocation is full";
        }
    });

    ft.wait();

    return 0;
}
