#ifndef _NODE_H_
#define _NODE_H_

#include <cstdint>

namespace bdd {
    // Public: the type of a BDD variable
    typedef uint32_t Variable;

    namespace internal {
        // Our internal BDD representation
        class Node {
            public:
                // SeemsGood -- Documented GCC/Clang builtins hack
                // TODO: to be consistent, shouldn't these be called true_node and false_node?
                static constexpr Node* true_bdd = __builtin_constant_p((Node*) 1) ? (Node*) 1 : (Node*) 1;
                static constexpr Node* false_bdd = __builtin_constant_p((Node*) 2) ? (Node*) 2 : (Node*) 2;

                // Uniquely identifying BDDs in canonical form
                bdd::Variable root;
                bool complemented; // Whether the edge ENTERING this node is complemented
                Node* branch_true;
                Node* branch_false;

                // Creates an uninitialized node that is marked as invalid.
                Node();
                // Creates node on stack, should be used in MK to get heap pointer
                Node(Variable root, bool complemented, Node* branch_true, Node* branch_false);

                static Node* make(bdd::Variable root, Node* branch_true, Node* branch_false);
                static Node* ITE(Node* A, Node* B, Node* C);

                // TODO: should any of these be static?
                static Node* evaluate_at(Node* node, bdd::Variable var, bool value);
                static Node* complement(Node* node); // Returns pointer to unique node with complemented
                static bool equals_complement(Node* A, Node* B); // TODO: this doesn't have to be static!

            private:
                // A reference count for freeing temporary BDDs
                uint32_t reference_count;
        };
    }
}

#endif
