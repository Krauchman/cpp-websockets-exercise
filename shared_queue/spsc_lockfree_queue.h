#pragma once

#include "queue_abstract.h"

#include <boost/lockfree/spsc_queue.hpp>

namespace shared_queue {
    namespace lockfree = boost::lockfree;

    template <typename T>
    class spsc_lockfree_queue : public shared_queue_base<T> {
    public:
        spsc_lockfree_queue(size_t element_count)
            : q_(element_count)
        {
        }

        bool push(const T& element) override {
            return q_.push(element);
        }

        T wait_pop() override {
            T element;
            while (!q_.pop(element));
            return element;
        }
    
    private:
        lockfree::spsc_queue<T> q_;
    };

} // shared_queue
