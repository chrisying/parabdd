#ifndef _NODESET_H_
#define _NODESET_H_

#include <list>

#include "node.h"

namespace bdd {
    namespace internal {

        // Lockfree set Kappa
        class NodeSet {
            public:
                Node* lookupOrCreate(const Node& value);
                bool init(size_t mem_usage);

            private:
                // Single-threaded implementation
                std::list<Node*> _set;
                Node* table;
        };
    }
}

#endif
