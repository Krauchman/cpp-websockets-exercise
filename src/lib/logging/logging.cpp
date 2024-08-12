#include "logging.h"

namespace logging {
    
    template <typename T>
    file_logger_base<T>::file_logger_base(std::string path)
        : buffer_(path)
        , out_(&buffer_)
    {
    }

    template <typename T>
    void file_logger_base<T>::log(const T& entry) {
        out_ << get_log_repr(entry) << std::endl;
    }

    csv_logger::csv_logger(std::string path)
        : file_logger_base<csv_row_convertible>(path)
    {
    }

    std::string csv_logger::get_log_repr(const csv_row_convertible& entry) {
        return entry.to_csv_row();
    }

} // logging
