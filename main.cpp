#include "coinbase/subscribe.h"
#include "coinbase/message.h"
#include "logging/logging.h"

#include "shared_queue/consumer.h"
#include "shared_queue/lock_queue.h"
#include "shared_queue/spsc_lockfree_queue.h"

#include <boost/log/trivial.hpp>
#include <boost/asio.hpp>

#include <thread>

namespace net = boost::asio;

using ticker_message = coinbase::message::ticker_message;

int main() {
    // shared queue
    auto q = std::make_shared<shared_queue::spsc_lockfree_queue<ticker_message>>(200);
    
    // logger thread
    logging::csv_logger logger("output.csv");
    shared_queue::looped_consumer<ticker_message> consumer(q);
    auto ft = std::async(std::launch::async, consumer, [&logger](const ticker_message& message) {
        logger.log(message);
    });

    // main thread processing websockets and parsing JSONs
    net::io_context ioc;
    coinbase::subscribe::ticker_subscriber subscriber(ioc);
    subscriber.start([q](const std::string& message) {
        if (!q->push({message})) {
            BOOST_LOG_TRIVIAL(error) << "Failed to push message to the queue: allocation is full";
        }
    });

    ft.wait();

    return 0;
}
