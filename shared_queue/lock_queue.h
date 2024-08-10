#pragma once

#include "queue_abstract.h"

#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>

#include <queue>

namespace shared_queue {

    template <typename T>
    class lock_queue : public shared_queue_base<T> {
    public:
        bool push(const T& element) override {
            boost::lock_guard<boost::mutex> lock(mtx_);
            q_.push(element);
            cond_non_empty_.notify_all();
            return true;
        }

        T wait_pop() override {
            boost::unique_lock<boost::mutex> lock(mtx_);
            cond_non_empty_.wait(lock, [&q = q_]() {
                return !q.empty();
            });
            T element = q_.front();
            q_.pop();
            return element;
        }
    
    private:
        boost::mutex mtx_;
        boost::condition_variable cond_non_empty_;
        std::queue<T> q_;
    };

} // shared_queue
