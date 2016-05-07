#include <cassert>
#include <iostream>

#include "hash.h"
#include "cache.h"

namespace bdd {
    namespace internal {
        static uint64_t hash(const CacheItem& data) {
            return hash128(&data, sizeof(CacheItem::key));
        }

        void Cache::init(size_t mem_usage) {
            _elems = mem_usage / sizeof(CacheSlot);
            _table = new CacheSlot[_elems];

            assert(_table != nullptr);
        }

        // A lock guard around nodes
        struct LockProtector {
            public:
                LockProtector(CacheSlot& slot) : _slot(slot) {
                    while (_slot.locked.test_and_set(std::memory_order_acquire));
                }
                ~LockProtector() {
                    _slot.locked.clear(std::memory_order_release);
                }

            private:
                CacheSlot& _slot;
        };

        void Cache::insert(const CacheItem& data) {
            uint64_t index = hash(data) % _elems;

            CacheSlot& current = _table[index];
            LockProtector lock(current);

            current.data = data;
            current.exists = true;
        }

        bool Cache::find(CacheItem& item) {
            uint64_t index = hash(item) % _elems;

            CacheSlot& current = _table[index];
            LockProtector lock(current);

            if (!current.exists) {
                return false;
            }

            if (current.data.key.a != item.key.a) {
                return false;
            }
            if (current.data.key.b != item.key.b) {
                return false;
            }
            if (current.data.key.c != item.key.c) {
                return false;
            }

            item.result = current.data.result;
            return true;
        }

        bool Cache::findITE(const NodePtr a, const NodePtr b, const NodePtr c, NodePtr& result) {
            CacheItem data;

            data.key.a = a;
            data.key.b = b;
            data.key.c = c;

            if (find(data)) {
                result = data.result;
                return true;
            }
            return false;
        }

        void Cache::insertITE(const NodePtr a, const NodePtr b, const NodePtr c, NodePtr result) {
            CacheItem data;

            data.key.a = a;
            data.key.b = b;
            data.key.c = c;
            data.result = result;

            insert(data);
        }
    }
}
