#include <cassert>

#include "cache.h"

namespace bdd {
    namespace internal {
        static uintptr_t hash64(uintptr_t key) {
            key = (~key) + (key << 21); // key = (key << 21) - key - 1;
            key = key ^ (key >> 24);
            key = (key + (key << 3)) + (key << 8); // key * 265
            key = key ^ (key >> 14);
            key = (key + (key << 2)) + (key << 4); // key * 21
            key = key ^ (key >> 28);
            key = key + (key << 31);
            return key;
        }

        static size_t hash(const CacheItem& data) {
            switch (data.tag) {
                case CacheItem::ITE: {
                    uintptr_t a = hash64(reinterpret_cast<uint64_t>(data.key.ITE.a));
                    uintptr_t b = hash64(reinterpret_cast<uint64_t>(data.key.ITE.b));
                    uintptr_t c = hash64(reinterpret_cast<uint64_t>(data.key.ITE.c));
                    return a ^ b ^ c;
                }

                case CacheItem::EvaluateAt: {
                    uintptr_t t = hash64(reinterpret_cast<uint64_t>(data.key.EvaluateAt.target));
                    uintptr_t v = hash64(data.key.EvaluateAt.var);
                    return t ^ v ^ data.key.EvaluateAt.value;
                }

                default:
                    assert(0);
            }
        }

        bool Cache::init(size_t mem_usage) {
            _elems = mem_usage / sizeof(CacheSlot);
            _table = new CacheSlot[_elems];

            return (_table != nullptr);
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
            size_t index = hash(data) % _elems;

            CacheSlot& current = _table[index];
            LockProtector lock(current);

            current.data = data;
        }

        bool Cache::find(CacheItem& item) {
            size_t index = hash(item) % _elems;

            CacheSlot& current = _table[index];
            LockProtector lock(current);

            if (current.data.tag != item.tag) {
                return false;
            }
            switch (item.tag) {
                case CacheItem::ITE:
                    if (current.data.key.ITE.a != item.key.ITE.a) {
                        return false;
                    }
                    if (current.data.key.ITE.b != item.key.ITE.b) {
                        return false;
                    }
                    if (current.data.key.ITE.c != item.key.ITE.c) {
                        return false;
                    }
                    break;

                case CacheItem::EvaluateAt:
                    if (current.data.key.EvaluateAt.target != item.key.EvaluateAt.target) {
                        return false;
                    }
                    if (current.data.key.EvaluateAt.var != item.key.EvaluateAt.var) {
                        return false;
                    }
                    if (current.data.key.EvaluateAt.value != item.key.EvaluateAt.value) {
                        return false;
                    }
                    break;
            }

            item.value = current.data.value;
            return true;
        }

        bool Cache::findITE(const Node* a, const Node* b, const Node* c, Node*& result) {
            CacheItem data;

            data.tag = CacheItem::ITE;
            data.key.ITE.a = a;
            data.key.ITE.b = b;
            data.key.ITE.c = c;

            if (find(data)) {
                result = data.value.ITE;
                return true;
            }
            return false;
        }

        bool Cache::findEvaluateAt(const Node* target, const Variable var, bool value, Node*& result) {
            CacheItem data;

            data.tag = CacheItem::EvaluateAt;
            data.key.EvaluateAt.target = target;
            data.key.EvaluateAt.var = var;
            data.key.EvaluateAt.value = value;

            if (find(data)) {
                result = data.value.EvaluateAt;
                return true;
            }
            return false;
        }

        void Cache::insertITE(const Node* a, const Node* b, const Node* c, Node* result) {
            CacheItem data;

            data.tag = CacheItem::ITE;
            data.key.ITE.a = a;
            data.key.ITE.b = b;
            data.key.ITE.c = c;
            data.value.ITE = result;

            insert(data);
        }

        void Cache::insertEvaluateAt(const Node* target, Variable var, bool value, Node* result) {
            CacheItem data;

            data.tag = CacheItem::EvaluateAt;
            data.key.EvaluateAt.target = target;
            data.key.EvaluateAt.var = var;
            data.key.EvaluateAt.value = value;
            data.value.EvaluateAt = result;

            insert(data);
        }
    }
}
