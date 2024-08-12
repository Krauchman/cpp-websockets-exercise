#include "config.h"

#include <boost/program_options.hpp>

namespace {
    namespace po = boost::program_options;
}

bool parse_config(int ac, char* av[], ticker_logger_config& config) {
    try {    
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help", "produce help message")
            ("output,o", po::value<std::string>(&config.output_file), "output file")
            ("product_id,p", po::value<std::vector<std::string>>(&config.product_ids), "product IDs (multiple)")
            ("use_lock_queue", po::bool_switch(&config.use_lock_queue),
                "Use regular queue with synchronization primitives instead of lock-free one")
        ;

        po::variables_map vm;
        po::store(po::parse_command_line(ac, av, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << "\n";
            return false;
        }

        if (!vm.count("output")) {
            throw std::logic_error("output file option is not provided");
        }

        if (config.product_ids.empty()) {
            throw std::logic_error("no product IDs provided");
        }
    }
    catch(std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return false;
    }
    catch(...) {
        std::cerr << "error: exception of unknown type\n";
        return false;
    }

    return true;
}
