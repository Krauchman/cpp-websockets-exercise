#include "coinbase/subscribe.h"
#include "coinbase/message.h"
#include "logging/logging.h"

#include "shared_queue/consumer.h"
#include "shared_queue/lock_queue.h"
#include "shared_queue/spsc_lockfree_queue.h"

#include <boost/log/trivial.hpp>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>

#include <thread>
#include <iostream>
#include <exception>

namespace net = boost::asio;
namespace po = boost::program_options;

using ticker_message = coinbase::message::ticker_message;

int main(int ac, char* av[]) {
    std::string output_file;
    bool use_lock_queue = false;
    std::vector<std::string> product_ids;

    try {

        po::options_description desc("Allowed options");
        desc.add_options()
            ("help", "produce help message")
            ("output,o", po::value<std::string>(&output_file), "output file")
            ("product_id,p", po::value<std::vector<std::string>>(&product_ids), "product IDs (multiple)")
            ("use_lock_queue", po::bool_switch(&use_lock_queue),
                "Use regular queue with synchronization primitives instead of lock-free one")
        ;

        po::variables_map vm;
        po::store(po::parse_command_line(ac, av, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << "\n";
            return 0;
        }

        if (!vm.count("output")) {
            throw std::logic_error("output file option is not provided");
        }

        if (product_ids.empty()) {
            throw std::logic_error("no product IDs provided");
        }

        if (!output_file.empty()) {
            BOOST_LOG_TRIVIAL(info) << "Output path: " << output_file;
        } else {
            BOOST_LOG_TRIVIAL(info) << "Output path was not set";
        }

        BOOST_LOG_TRIVIAL(info) << "Number of product IDs to subscribe to: " << product_ids.size();

        if (use_lock_queue) {
            BOOST_LOG_TRIVIAL(info) << "Thead-shared queue implementation: locking queue";
        } else {
            BOOST_LOG_TRIVIAL(info) << "Thead-shared queue implementation: lock-free queue";
        }
    }
    catch(std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
    catch(...) {
        std::cerr << "Exception of unknown type!\n";
    }

    // shared queue
    std::shared_ptr<shared_queue::shared_queue_base<ticker_message>> q;
    if (use_lock_queue) {
        q = std::make_shared<shared_queue::lock_queue<ticker_message>>();
    } else {
        q = std::make_shared<shared_queue::spsc_lockfree_queue<ticker_message>>(200);
    }
    
    // logger thread
    logging::csv_logger logger(output_file);
    shared_queue::looped_consumer<ticker_message> consumer(q);
    auto ft = std::async(std::launch::async, consumer, [&logger](const ticker_message& message) {
        logger.log(message);
    });

    // main thread processing websockets and parsing JSONs
    net::io_context ioc;
    coinbase::subscribe::ticker_subscriber subscriber(ioc, product_ids);
    subscriber.start([q](const std::string& message) {
        if (!q->push({message})) {
            BOOST_LOG_TRIVIAL(error) << "Failed to push message to the queue: allocation is full";
        }
    });

    ft.wait();

    return 0;
}
