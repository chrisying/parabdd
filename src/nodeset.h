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
                NodeSet(size_t num_elems);

            private:
                // Single-threaded implementation
                std::list<Node*> _set;
        };
    }
}

#endif
