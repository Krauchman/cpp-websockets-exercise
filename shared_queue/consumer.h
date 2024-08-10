#pragma once

#include "queue_abstract.h"

#include <memory>
#include <functional>

namespace shared_queue {

    template <typename T>
    class looped_consumer {
    public:
        looped_consumer(std::shared_ptr<shared_queue_base<T>> q_ptr)
            : q_ptr_(q_ptr)
        {
        }

        void operator()(std::function<void(const T&)> callback) {
            while (true) {
                callback(q_ptr_->wait_pop());
            }
        }

    private:
        std::shared_ptr<shared_queue_base<T>> q_ptr_;
    };

} // shared_queue
