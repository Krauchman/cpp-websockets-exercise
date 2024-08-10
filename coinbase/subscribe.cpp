#include "subscribe.h"

#include <boost/log/trivial.hpp>
#include <boost/json.hpp>

namespace coinbase::subscribe {
    namespace json = boost::json;

    ticker_subscriber::ticker_subscriber(net::io_context& ioc)
        : session_("ws-feed.exchange.coinbase.com", "443", ioc)
    {
    }

    void ticker_subscriber::start(std::function<void(const std::string&)> callback) {
        session_.start();

        subscribe();

        while (true) {
            session_.read(buffer_);
            callback(beast::buffers_to_string(buffer_.data()));
            buffer_.consume(buffer_.size());
        }
    }

    void ticker_subscriber::subscribe() {
        json::value subscribe_message_data = {
            { "type", "subscribe" },
            { "product_ids", json::array{"ETH-USD", "BTC-USD"} },
            { "channels", json::array{"ticker"} }
        };
        std::string subscribe_message = json::serialize(subscribe_message_data);
        
        session_.write(subscribe_message);

        session_.read(buffer_);
        BOOST_LOG_TRIVIAL(info) << "Subscribe response from the exchange: " << beast::make_printable(buffer_.data());
        buffer_.consume(buffer_.size());
    }

} // coinbase::subscribe
