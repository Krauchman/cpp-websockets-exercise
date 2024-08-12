#include "subscribe.h"

#include <boost/log/trivial.hpp>
#include <nlohmann/json.hpp>

namespace coinbase::subscribe {
    using json = nlohmann::json;

    ticker_subscriber::ticker_subscriber(net::io_context& ioc, std::vector<std::string> product_ids)
        : session_(std::make_unique<websocket::session>("ws-feed.exchange.coinbase.com", "443", ioc))
        , product_ids_(std::move(product_ids))
    {
    }

    ticker_subscriber::ticker_subscriber(std::unique_ptr<websocket::session_base> session, std::vector<std::string> product_ids)
        : session_(std::move(session))
        , product_ids_(std::move(product_ids))
    {
    }

    void ticker_subscriber::start(std::function<void(const std::string&)> callback) {
        session_->start();

        if (!subscribe()) {
            throw std::runtime_error("Failed to subscribe to a ticker channel");
        }

        BOOST_LOG_TRIVIAL(info) << "Listening for messages";
        while (true) {
            callback(session_->read());
        }
    }

    bool ticker_subscriber::subscribe() {
        json subscribe_message_data = {
            { "type", "subscribe" },
            { "product_ids", product_ids_ },
            { "channels", {"ticker"} }
        };
        std::string subscribe_message = subscribe_message_data.dump();
        
        session_->write(subscribe_message);

        auto response = session_->read();
        auto response_json = json::parse(response);
        if (response_json["type"] != "subscriptions") {
            BOOST_LOG_TRIVIAL(error) << "Got unexpected response from the exchange: " << response;
            return false;
        }
        
        BOOST_LOG_TRIVIAL(info) << "Successfully subscribed to a ticker channel, response from the exchange: " << response;

        return true;
    }

} // coinbase::subscribe
