#ifndef _NODESET_H_
#define _NODESET_H_

#include <cstddef>

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
                Node* lookupOrCreate(const Node& value);
                bool init(size_t mem_usage);

            private:
                size_t _elems;
                std::atomic<size_t> _count;
                NodeSlot* _table;
        };
    }
}

#endif
