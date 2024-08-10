#pragma once

#include <logging/logging.h>

#include <nlohmann/json.hpp>

namespace coinbase::message {
    using json = nlohmann::json;

    class message_base : public logging::csv_row_convertible {
    public:
        message_base() {}
        message_base(std::string message_str);

        std::string to_csv_row() const override;

        virtual std::vector<std::string> get_field_names() const = 0;

    private:
        json data_;
    };

    class ticker_message : public message_base {
    public:
        ticker_message() : message_base() {}
        ticker_message(std::string message_str);

        std::vector<std::string> get_field_names() const override;
    };
    
} // coinbase::message
