#ifndef _NODESET_H_
#define _NODESET_H_

#include <cstddef>
#include <atomic>

#include "node.h"

namespace bdd {
    namespace internal {
        struct NodeSlot {
            std::atomic_flag locked;
            bool exists;

            Node node;
        };

        class NodeSet {
            public:
                NodePtr lookupOrCreate(const Node& value);
                void init(size_t mem_usage);
                NodeSlot* table;

                std::atomic<uint32_t> _count;
            private:
                uint32_t _elems;
        };
    }
}

#endif
