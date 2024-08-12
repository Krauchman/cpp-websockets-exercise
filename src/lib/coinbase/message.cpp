#include "message.h"

namespace coinbase::message {

    message_base::message_base(std::string message_str)
        : data_(std::move(json::parse(std::move(message_str))))
    {
    }

    std::string message_base::to_csv_row() const {
        std::string result;
        auto field_names = get_field_names();
        for (size_t i = 0; i < field_names.size(); ++i) {
            if (i) {
                result.push_back(',');
            }
            const auto& field_name = field_names[i];
            if (data_.contains(field_name)) {
                result += data_[field_name].dump();
            }
        }
        return result;
    }

    std::string message_base::to_csv_header() const {
        std::string result;
        auto field_names = get_field_names();
        for (size_t i = 0; i < field_names.size(); ++i) {
            if (i) {
                result.push_back(',');
            }
            result += field_names[i];
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
