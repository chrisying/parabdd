#ifndef _NODESET_H_
#define _NODESET_H_

#include <cstddef>

#include "node.h"

namespace bdd {
    namespace internal {
        struct NodeSlot {
            std::atomic<bool> locked;
            Node node;

            NodeSlot() : locked(false) { }
        };

        class NodeSet {
            public:
                Node* lookupOrCreate(const Node& value);
                bool init(size_t mem_usage);

            private:
                size_t _elems;
                NodeSlot* _table;
        };
    }
}

#endif
