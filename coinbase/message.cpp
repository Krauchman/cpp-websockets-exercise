#include "message.h"

namespace coinbase::message {

    message_base::message_base(std::string message_str)
        : data_(std::move(json::parse(message_str)))
    {
    }

    std::string message_base::to_csv_row() const {
        std::string result;
        for (auto field_name : get_field_names()) {
            auto value_ptr = data_.as_object().if_contains(field_name);

            if (value_ptr) {
                result += json::serialize(*value_ptr);
            }
            result.push_back(',');
        }
        return result;
    }

    ticker_message::ticker_message(std::string message_str) 
        : message_base(message_str)
    {
    }

    std::vector<std::string> ticker_message::get_field_names() const {
        return {"type", "sequence", "product_id", "price",
                "open_24h", "volume_24h", "low_24h",
                "high_24h", "volume_30d", "best_bid",
                "best_bid_size", "best_ask", "best_ask_size",
                "side", "time", "trade_id", "last_size"};
    }

} // coinbase::message
