#include "coinbase/subscribe.h"
#include "coinbase/message.h"
#include "logging/logging.h"

#include <boost/lockfree/spsc_queue.hpp>
#include <boost/log/trivial.hpp>
#include <boost/asio.hpp>

#include <thread>

namespace net = boost::asio;
namespace lockfree = boost::lockfree;

namespace queue {
    
    template <typename T>
    class looped_consumer {
    public:
        looped_consumer(std::shared_ptr<lockfree::spsc_queue<T>> q_ptr)
            : q_ptr_(q_ptr)
        {
        }

        void operator()(std::function<void(const T&)> callback) {
            while (true) {
                T message;
                while (!q_ptr_->pop(message));

                callback(message);
            }
        }
    private:
        std::shared_ptr<lockfree::spsc_queue<T>> q_ptr_;
    };

} // queue

int main() {
    // shared queue
    auto q = std::make_shared<lockfree::spsc_queue<coinbase::message::ticker_message>>(200);
    
    // logger thread
    logging::csv_logger logger("output.csv");
    queue::looped_consumer consumer(q);
    auto ft = std::async(std::launch::async, consumer, [&logger](const coinbase::message::ticker_message& message) {
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
