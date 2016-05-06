#ifndef _NODESET_H_
#define _NODESET_H_

#include <cstddef>

#include "node.h"

namespace bdd {
    namespace internal {
        struct CacheItem {
            enum {
                ITE,
                EvaluateAt
            } tag;
            union {
                struct {
                    const Node* a;
                    const Node* b;
                    const Node* c;
                } ITE;
                struct {
                    const Node* target;
                    Variable var;
                    bool value;
                } EvaluateAt;
            } key;
            union {
                Node* ITE;
                Node* EvaluateAt;
            } value;
        };

        struct CacheSlot {
            std::atomic<bool> locked;
            CacheItem data;

            CacheSlot() : locked(false) { }
        };

        class Cache {
            public:
                bool findITE(const Node* a, const Node* b, const Node* c, Node*& result);
                bool findEvaluateAt(const Node* target, const Variable var, bool value, Node*& result);

                void insertITE(const Node* a, const Node* b, const Node* c, Node* result);
                void insertEvaluateAt(const Node* target, Variable var, bool value, Node* result);

                bool init(size_t mem_usage);

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
