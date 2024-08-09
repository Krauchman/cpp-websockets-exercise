#pragma once

#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/stream.hpp>

#include <ostream>

namespace logging {
    namespace io = boost::iostreams;
    
    template <typename T>
    class logger_base {
    public:
        virtual void log(const T& entry) = 0;
    };

    template <typename T>
    class file_logger_base : public logger_base<T> {
    public:
        file_logger_base(std::string path);

        void log(const T& entry) override;

    protected:
        virtual std::string get_log_repr(const T& entry) = 0;

        io::stream_buffer<io::file_sink> buffer_;
        std::ostream out_;
    };

    class csv_row_convertible {
    public:
        virtual std::string to_csv_row() const = 0;
    };

    class csv_logger : public file_logger_base<csv_row_convertible> {
    public:
        csv_logger(std::string path);
    
    protected:
        std::string get_log_repr(const csv_row_convertible& entry) override;
    };

} // logging
