#pragma once

#include <vector>
#include <string>
#include <iostream>

struct ticker_logger_config {
    std::string output_file;
    std::vector<std::string> product_ids;
    bool use_lock_queue = false;
};

bool parse_config(int ac, char* av[], ticker_logger_config& config);
