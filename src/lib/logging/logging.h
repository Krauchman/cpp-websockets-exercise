#pragma once

#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/stream.hpp>

#include <ostream>

namespace logging {
    namespace io = boost::iostreams;
    
    template <typename T>
    class logger_base {
    public:
        virtual ~logger_base() = default;

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
        virtual ~csv_row_convertible() = default;

        virtual std::string to_csv_row() const = 0;
        virtual std::string to_csv_header() const = 0;
    };

    class csv_logger : public file_logger_base<csv_row_convertible> {
    public:
        csv_logger(std::string path);

        template <typename T>
        void log_header() {
            T dummy;
            out_ << dummy.to_csv_header() << std::endl;
        }
    
    protected:
        std::string get_log_repr(const csv_row_convertible& entry) override;
    };

} // logging
