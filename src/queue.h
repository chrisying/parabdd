/* Lock Free Multiproducer Multiconsumer Bounded Queue
 *
 * Strongly based off of the implementation at
 * http://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue,
 */

#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <atomic>
#include <thread>

template<typename T> class Queue {
    public:
        Queue(size_t buffer_size) : mask(buffer_size - 1), buffer(new cell[buffer_size]) {
            // Buffer size must be a power of 2
            assert((buffer_size >= 2) && ((buffer_size & mask) == 0));

            for (size_t i = 0; i != buffer_size; i += 1) {
                buffer[i].sequence.store(i, std::memory_order_relaxed);
            }

            tail.store(0, std::memory_order_relaxed);
            head.store(0, std::memory_order_relaxed);
        }

        bool enqueue(T const& data) {
            cell* cell = nullptr;

            size_t pos = tail.load(std::memory_order_relaxed);

            while (true) {
                cell = &buffer[pos & mask];
                size_t seq = cell->sequence.load(std::memory_order_acquire);
                ssize_t dif = (ssize_t) seq - (ssize_t) pos;

                if (dif == 0) {
                    if (tail.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
                        break;
                    }
                }
                else if (dif < 0) {
                    return false;
                }
                else {
                    pos = tail.load(std::memory_order_relaxed);
                }
            }

            cell->data = data;
            cell->sequence.store(pos + 1, std::memory_order_release);

            return true;
        }

        bool dequeue(T& data) {
            cell* cell = nullptr;

            size_t pos = head.load(std::memory_order_relaxed);

            while (true) {
                cell = &buffer[pos & mask];
                size_t seq = cell->sequence.load(std::memory_order_acquire);
                ssize_t dif = (ssize_t) seq - (ssize_t) (pos + 1);

                if (dif == 0) {
                    if (head.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
                        break;
                    }
                }
                else if (dif < 0) {
                    return false;
                }
                else {
                    pos = head.load(std::memory_order_relaxed);
                }
            }

            data = cell->data;
            cell->sequence.store(pos + mask + 1, std::memory_order_release);

            return true;
        }

    private:
        struct cell {
            std::atomic<size_t> sequence;
            T data;
        };

        const size_t mask;
        static constexpr size_t cache_line = 64;

        cell* const buffer;
        alignas(cache_line) std::atomic<size_t> tail;
        alignas(cache_line) std::atomic<size_t> head;
};

#endif
