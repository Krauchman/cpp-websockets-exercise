#include "message.h"

#include <exception>

namespace coinbase::message {
    const std::string TERMINATION_FIELD_NAME = "termination";

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

    bool message_base::is_terminational() const {
        return data_.contains(TERMINATION_FIELD_NAME) && data_[TERMINATION_FIELD_NAME].dump() == "true";
    }

    void message_base::make_terminational() {
        data_[TERMINATION_FIELD_NAME] = true;
    }

    ticker_message::ticker_message(std::string message_str)
        : message_base(std::move(message_str))
    {
        if (!data_.contains("type") || data_["type"].dump() != "\"ticker\"") {
            throw std::logic_error("Failed to initialize ticker message: data type is not \"ticker\"");
        }
    }

    std::vector<std::string> ticker_message::get_field_names() const {
        return {"type", "sequence", "product_id", "price",
                "open_24h", "volume_24h", "low_24h",
                "high_24h", "volume_30d", "best_bid",
                "best_bid_size", "best_ask", "best_ask_size",
                "side", "time", "trade_id", "last_size"};
    }

} // coinbase::message
