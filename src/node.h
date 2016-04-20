#ifndef _NODE_H_
#define _NODE_H_

#include <cstdint>
#include <atomic>

namespace bdd {
    // Public: the type of a BDD variable
    typedef uint32_t Variable;

    namespace internal {
        // Our internal BDD representation
        // A pointer to a Node will have lowest order bit set to 1 if it is complemented
        class Node {
            public:
                // SeemsGood -- Documented GCC/Clang builtins hack
                static constexpr Node* true_node = __builtin_constant_p((Node*) 1) ? (Node*) 3 : (Node*) 3;  // 3 == 0b11 == compl(0b10) == compl(2)
                static constexpr Node* false_node = __builtin_constant_p((Node*) 0) ? (Node*) 2 : (Node*) 2;

                // A reference count for freeing temporary BDDs
                std::atomic<uint32_t> reference_count;

                // Uniquely identifying BDDs in canonical form
                bdd::Variable root;
                Node* branch_true;
                Node* branch_false;

                // Creates an uninitialized node that is marked as invalid.
                Node();
                // Creates node on stack, should be ONLY used in make to get heap pointer
                Node(Variable root, Node* branch_true, Node* branch_false);

                // Creates node on the heap, this is the pointer that should be used in other operations
                static Node* make(bdd::Variable root, Node* branch_true, Node* branch_false);
                static Node* ITE(Node* A, Node* B, Node* C);
                static Node* evaluate_at(Node* node, bdd::Variable var, bool value);
                static Node* complement(Node* node); // Returns pointer to unique node with complemented

                static inline bool is_complemented(Node* node);
                static inline bool equals_complement(Node* A, Node* B);
        };
    }
}

#endif
