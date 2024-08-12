#pragma once

namespace shared_queue {
    
    template <typename T>
    class shared_queue_base {
    public:
        virtual ~shared_queue_base() = default;

        virtual bool push(const T& element) = 0;
        virtual T wait_pop() = 0;
    };

} // shared_queue
