#ifndef _CACHE_H_
#define _CACHE_H_

#include <cstddef>
#include <atomic>

#include "node.h"

namespace bdd {
    namespace internal {
        struct CacheItem {
            struct {
                NodePtr a;
                NodePtr b;
                NodePtr c;
            } key;
            NodePtr result;
        };

        struct CacheSlot {
            std::atomic_flag locked;
            bool exists;

            CacheItem data;

            CacheSlot() : locked(false) { }
        };

        class Cache {
            public:
                bool findITE(const NodePtr a, const NodePtr b, const NodePtr c, NodePtr& result);
                void insertITE(const NodePtr a, const NodePtr b, const NodePtr c, NodePtr result);

                void init(size_t mem_usage);

            private:
                size_t _elems;
                size_t _count;
                CacheSlot* _table;

                bool find(CacheItem& data);
                void insert(const CacheItem& data);
        };
    }
}

#endif
