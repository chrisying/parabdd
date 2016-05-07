#include <cassert>
#include <limits>
#include <cstdint>
#include <iostream>
#include <string>
#include <set>

#include "bdd.h"

namespace bdd {
    namespace internal {
        static inline Node* evaluate_at(Node* node, Variable var, bool value);
        static inline Variable top_variable(Node* A, Node* B, Node* C);
        static inline Node* complement(Node* node);
        static inline bool is_complemented(Node* node);
        static inline bool equals_complement(Node* A, Node* B);
        static inline bool is_leaf(Node* node);
        static inline Node* pointer(Node* node);

        static inline Node* evaluate_at(Node* node, Variable var, bool value) {
            // Variable is above this node, nothing changes
            if (is_leaf(node) || pointer(node)->root > var) {
                return node;
            }

            // Variable is exactly this node, choose appropriate branch
            if (pointer(node)->root == var) {
                Node* target = (value ? pointer(node)->branch_true : pointer(node)->branch_false);
                return is_complemented(node) ? complement(target) : target;
            }
            assert(false); // TODO: remove if we use evaluate in anything except ITE

            // Check cache
            Node* new_node;
            if (manager::cache.findEvaluateAt(node, var, value, new_node)) {
                return new_node;
            }

            new_node = Node::make(var, evaluate_at(pointer(node)->branch_true, var, value), evaluate_at(pointer(node)->branch_false, var, value));
            new_node = is_complemented(node) ? complement(new_node) : new_node;

            // Put new_node in cache
            manager::cache.insertEvaluateAt(node, var, value, new_node);

            return new_node;
        }

        static inline Variable top_variable(Node* A, Node* B, Node* C) {
            auto var = [] (Node* x) {
                return (is_leaf(x) ? std::numeric_limits<Variable>::max() : pointer(x)->root);
            };

            return std::min(var(A), std::min(var(B), var(C)));
        }

        // Inverts the lowest order bit
        static inline Node* complement(Node* node) {
            return reinterpret_cast<Node*>(((uintptr_t) node) ^ 0x1);
        }

        static inline bool is_complemented(Node* node) {
            return (((uintptr_t) node) & 0x1) == 0x1;
        }

        // True iff A and B are the same except the lowest order bit
        static inline bool equals_complement(Node* A, Node* B) {
            return A == complement(B);
        }

        static inline bool is_leaf(Node* node) {
            return node == Node::true_node || node == Node::false_node;
        }

        // Sets lowest order bit to 0
        static inline Node* pointer(Node* node) {
            return reinterpret_cast<Node*>(((uintptr_t) node) & ((uintptr_t) ~0x1LL));
        }

        Node::Node() : reference_count(Node::unused) { }

        Node::Node(Variable root, Node* branch_true, Node* branch_false) : root(root), branch_true(branch_true), branch_false(branch_false) { }

        Node* Node::make(Variable root, Node* branch_true, Node* branch_false) {
            if (branch_true == branch_false) {
                return branch_false;
            }

            // Enforce canonicity (complement only on 1 edge)
            if (is_complemented(branch_false)) {
                Node* new_true = complement(branch_true);
                Node* new_false = complement(branch_false);
                Node node(root, new_true, new_false);
                return complement(manager::nodes.lookupOrCreate(node));
            }

            Node node(root, branch_true, branch_false);
            return manager::nodes.lookupOrCreate(node);
        }

        Node* Node::ITE(Node* A, Node* B, Node* C) {
            // Base cases
            if (A == true_node) { return B; }
            if (A == false_node) { return C; }
            if (B == true_node && C == false_node) { return A; }
            if (B == false_node && C == true_node) { return complement(A); }
            if (B == C) { return B; }

            // Check if this ITE has been done before in cache
            Node* result;
            if (manager::cache.findITE(A, B, C, result)) {
                return result;
            }

            // Normalization rules
            if (A == B) {
                // ITE(A,A,C) -> ITE(A,1,C)
                result = ITE(A, true_node, C);
            } else if (equals_complement(A, B)) {
                // ITE(A,!A,C) -> ITE(A,0,C)
                result = ITE(A, false_node, C);
            } else if (A == C) {
                // ITE(A,B,A) -> ITE(A,B,0)
                result = ITE(A, B, false_node);
            } else if (equals_complement(A, C)) {
                // ITE(A,B,!A) -> ITE(A,B,1)
                result = ITE(A, B, true_node);
            } else if (B == true_node && pointer(C)->root < pointer(A)->root) {
                // ITE(A,1,C) -> ITE(C,1,A) if C < A
                result = ITE(C, true_node, A);
            } else if (B == false_node && pointer(C)->root < pointer(A)->root) {
                // ITE(A,0,C) -> ITE(!C,0,!A) if C < A
                result = ITE(complement(C), false_node, complement(A));
            } else if (C == true_node && pointer(B)->root < pointer(A)->root) {
                // ITE(A,B,1) -> ITE(!B,!A,1) if B < A
                result = ITE(complement(B), complement(A), true_node);
            } else if (C == false_node && pointer(B)->root < pointer(A)->root) {
                // ITE(A,B,0) -> ITE(B,A,0) if B < A
                result = ITE(B, A, false_node);
            } else if (equals_complement(B, C) && pointer(B)->root < pointer(C)->root) {
                // ITE(A,B,!B) -> ITE(B,A,!A)
                result = ITE(B, A, complement(A));
            } else if (is_complemented(A)) {
                // ITE(A,B,C) -> ITE(!A,C,B) if A complemented
                result = ITE(complement(A), C, B);
            } else if (is_complemented(B)) {
                // ITE(A,B,C) -> !ITE(A,!B,!C) if B complemented
                result = complement(ITE(A, complement(B), complement(C)));
            } else {
                // If no normalization applies
                Variable x = top_variable(A, B, C);
                Node* A_false = evaluate_at(A, x, false);
                Node* B_false = evaluate_at(B, x, false);
                Node* C_false = evaluate_at(C, x, false);
                Node* A_true = evaluate_at(A, x, true);
                Node* B_true = evaluate_at(B, x, true);
                Node* C_true = evaluate_at(C, x, true);

                Node* R_false = cilk_spawn ITE(A_false, B_false, C_false);
                Node* R_true = cilk_spawn ITE(A_true, B_true, C_true);

                cilk_sync;

                result = make(x, R_true, R_false);
            }

            // Put in cache
            manager::cache.insertITE(A, B, C, result);

            return result;
        }

        static uintptr_t qp(Node* n) {
            return reinterpret_cast<uintptr_t>(n);
        }


#define ID(uniq, x) "\"" << uniq << "_" << x << "\""
#define IDQ(uniq, node) ID(uniq, qp(node))

        static void print_rec(Node* node, std::set<Node*>& visited, uintptr_t uniq) {
            if (visited.count(node)) {
                return;
            }
            if (is_leaf(node)) {
                return;
            }

            std::cout << IDQ(uniq, node) << " [label=\"" << node->root << "\"];\n";

            std::cout << IDQ(uniq, node) << " -> " << IDQ(uniq, pointer(node->branch_false)) << " [style=dashed];\n";
            std::cout << IDQ(uniq, node) << " -> " << IDQ(uniq, pointer(node->branch_true)) << " [style=filled]" << (is_complemented(node->branch_true) ? "[color=red]" : "") << ";\n";

            visited.insert(node);

            print_rec(pointer(node->branch_true), visited, uniq);
            print_rec(pointer(node->branch_false), visited, uniq);
        }

        void Node::print(Node* node, std::string title) {
            uintptr_t uniq = qp(node);

            std::cout << "digraph \"G_" << uniq << "\" {\n";
            std::cout << "labelloc=\"t\";\n";
            std::cout << "label=\"" << title << "\";\n";

            std::cout << IDQ(uniq, false_node) << " [shape=box, label=\"false\", style=filled, height=0.3, width=0.3];\n";

            std::cout << ID(uniq, "f") << " [shape=triangle, label=\"f\", style=filled, height=0.3, width=0.3];\n";
            std::cout << ID(uniq, "f") " -> " << IDQ(uniq, pointer(node)) << " [style=filled]" << (is_complemented(node) ? "[color=red]" : "") << ";\n";

            std::set<Node*> visited;
            print_rec(pointer(node), visited, uniq);

            std::cout << "}\n";
        }
    }
}
