#include "subscribe.h"

#include <boost/log/trivial.hpp>
#include <nlohmann/json.hpp>

namespace coinbase::subscribe {
    using json = nlohmann::json;

    ticker_subscriber::ticker_subscriber(net::io_context& ioc, std::vector<std::string> product_ids)
        : session_("ws-feed.exchange.coinbase.com", "443", ioc)
        , product_ids_(product_ids)
    {
    }

    void ticker_subscriber::start(std::function<void(const std::string&)> callback) {
        session_.start();

        if (!subscribe()) {
            throw std::runtime_error("Failed to subscribe to a ticker channel");
        }

        while (true) {
            session_.read(buffer_);
            callback(beast::buffers_to_string(buffer_.data()));
            buffer_.consume(buffer_.size());
        }
    }

    bool ticker_subscriber::subscribe() {
        json subscribe_message_data = {
            { "type", "subscribe" },
            { "product_ids", product_ids_ },
            { "channels", {"ticker"} }
        };
        std::string subscribe_message = subscribe_message_data.dump();
        
        session_.write(subscribe_message);

        session_.read(buffer_);
        
        auto response_json = json::parse(beast::buffers_to_string(buffer_.data()));
        if (response_json["type"] != "subscriptions") {
            BOOST_LOG_TRIVIAL(error) << "Got unexpected response from the exchange: " << beast::make_printable(buffer_.data());
            return false;
        }
        
        BOOST_LOG_TRIVIAL(info) << "Successfully subscribed to a ticker channel, response from the exchange: " << beast::make_printable(buffer_.data());

        buffer_.consume(buffer_.size());
        return true;
    }

} // coinbase::subscribe
