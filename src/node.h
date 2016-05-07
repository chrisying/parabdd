#ifndef _NODE_H_
#define _NODE_H_

#include <cstdint>
#include <limits>
#include <string>

namespace bdd {
    // Public: the type of a BDD variable
    typedef uint32_t Variable;

    namespace internal {
        // Our internal BDD representation
        // A pointer to a Node will have the highest order bit set to 1 if it is complemented
        typedef uint32_t NodePtr;

        class Node {
            public:
                static constexpr NodePtr true_node = 0x80000000;
                static constexpr NodePtr false_node = 0x00000000;

                // Uniquely identifying BDDs in canonical form
                NodePtr branch_true;
                NodePtr branch_false;
                bdd::Variable root;

                // Creates an uninitialized node that is marked as invalid.
                Node();
                // Creates node on stack, should be ONLY used in make to get heap pointer
                Node(Variable root, NodePtr branch_true, NodePtr branch_false);

                // Creates node on the heap, this is the pointer that should be used in other operations
                static NodePtr make(bdd::Variable root, NodePtr branch_true, NodePtr branch_false);
                static NodePtr ITE(NodePtr A, NodePtr B, NodePtr C);

                // For debug purposes, will print a full graph of the tree
                static void print(NodePtr node, std::string title);
        };
    }
}

#endif
