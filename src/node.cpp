#include <cassert>
#include <limits>
#include <cstdint>
#include <iostream>
#include <string>
#include <set>

#include "bdd.h"
#include "parallel.h"

namespace bdd {
    namespace internal {
        static inline NodePtr evaluate_at(NodePtr node, Variable var, bool value);
        static inline Variable top_variable(NodePtr A, NodePtr B, NodePtr C);
        static inline NodePtr complement(NodePtr node);
        static inline bool is_complemented(NodePtr node);
        static inline bool equals_complement(NodePtr A, NodePtr B);
        static inline bool is_leaf(NodePtr node);
        static inline Node* pointer(NodePtr node);

        static inline NodePtr evaluate_at(NodePtr node, Variable var, bool value) {
            // Variable is above this node, nothing changes
            if (is_leaf(node) || pointer(node)->root > var) {
                return node;
            }

            // Variable is exactly this node, choose appropriate branch
            if (pointer(node)->root == var) {
                NodePtr target = (value ? pointer(node)->branch_true : pointer(node)->branch_false);
                return is_complemented(node) ? complement(target) : target;
            }
            assert(false); // TODO: remove if we use evaluate in anything except ITE

            NodePtr new_node = Node::make(var, evaluate_at(pointer(node)->branch_true, var, value), evaluate_at(pointer(node)->branch_false, var, value));
            new_node = is_complemented(node) ? complement(new_node) : new_node;

            return new_node;
        }

        static inline Variable top_variable(NodePtr A, NodePtr B, NodePtr C) {
            auto var = [] (NodePtr x) {
                return (is_leaf(x) ? std::numeric_limits<Variable>::max() : pointer(x)->root);
            };

            return std::min(var(A), std::min(var(B), var(C)));
        }

        static inline NodePtr complement(NodePtr node) {
            return node ^ 0x80000000;
        }

        static inline bool is_complemented(NodePtr node) {
            return (node & 0x80000000);
        }

        static inline bool equals_complement(NodePtr A, NodePtr B) {
            return A == complement(B);
        }

        static inline bool is_leaf(NodePtr node) {
            return node == Node::true_node || node == Node::false_node;
        }

        static inline Node* pointer(NodePtr node) {
            uint32_t index = 0x7FFFFFFF & node;
            return &manager::nodes.table[index].node;
        }

        Node::Node() { }

        Node::Node(Variable root, NodePtr branch_true, NodePtr branch_false) : branch_true(branch_true), branch_false(branch_false), root(root) { }

        NodePtr Node::make(Variable root, NodePtr branch_true, NodePtr branch_false) {
            if (branch_true == branch_false) {
                return branch_false;
            }

            // Enforce canonicity (complement only on 1 edge)
            if (is_complemented(branch_false)) {
                NodePtr new_true = complement(branch_true);
                NodePtr new_false = complement(branch_false);
                Node node(root, new_true, new_false);
                return complement(manager::nodes.lookupOrCreate(node));
            }

            Node node(root, branch_true, branch_false);
            return manager::nodes.lookupOrCreate(node);
        }

        NodePtr Node::ITE(NodePtr A, NodePtr B, NodePtr C) {
            // Base cases
            if (A == true_node) { return B; }
            if (A == false_node) { return C; }
            if (B == true_node && C == false_node) { return A; }
            if (B == false_node && C == true_node) { return complement(A); }
            if (B == C) { return B; }

            // Check if this ITE has been done before in cache
            NodePtr result;
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
                NodePtr A_false = evaluate_at(A, x, false);
                NodePtr B_false = evaluate_at(B, x, false);
                NodePtr C_false = evaluate_at(C, x, false);
                NodePtr A_true = evaluate_at(A, x, true);
                NodePtr B_true = evaluate_at(B, x, true);
                NodePtr C_true = evaluate_at(C, x, true);

                NodePtr R_false = parallel ITE(A_false, B_false, C_false);
                NodePtr R_true = parallel ITE(A_true, B_true, C_true);

                syncpoint;

                result = make(x, R_true, R_false);
            }

            // Put in cache
            manager::cache.insertITE(A, B, C, result);

            return result;
        }


#define ID(uniq, x) "\"" << uniq << "_" << x << "\""

        static void print_rec(NodePtr node, std::set<NodePtr>& visited, NodePtr uniq) {
            node = (is_complemented(node) ? complement(node) : node);
            if (visited.count(node)) {
                return;
            }
            if (is_leaf(node)) {
                return;
            }
            Node* p = pointer(node);

            std::cout << ID(uniq, node) << " [label=\"" << p->root << "\"];\n";

            std::cout << ID(uniq, node) << " -> " << ID(uniq, p->branch_false) << " [style=dashed];\n";
            if (is_complemented(p->branch_true)) {
                std::cout << ID(uniq, node) << " -> " << ID(uniq, complement(p->branch_true)) << " [style=filled] [color=red];\n";
            }
            else {
                std::cout << ID(uniq, node) << " -> " << ID(uniq, p->branch_true) << " [style=filled];\n";
            }

            visited.insert(node);

            print_rec(p->branch_true, visited, uniq);
            print_rec(p->branch_false, visited, uniq);
        }

        void Node::print(NodePtr node, std::string title) {
            std::cout << "digraph \"G_" << node << "\" {\n";
            std::cout << "labelloc=\"t\";\n";
            std::cout << "label=\"" << title << "\";\n";

            std::cout << ID(node, false_node) << " [shape=box, label=\"false\", style=filled, height=0.3, width=0.3];\n";

            std::cout << ID(node, "f") << " [shape=triangle, label=\"f\", style=filled, height=0.3, width=0.3];\n";
            if (is_complemented(node)) {
                std::cout << ID(node, "f") << " -> " << ID(node, complement(node)) << " [style=filled] [color=red];\n";
            }
            else {
                std::cout << ID(node, "f") << " -> " << ID(node, node) << " [style=filled];\n";
            }

            std::set<NodePtr> visited;
            print_rec(node, visited, node);

            std::cout << "}\n";
        }
    }
}
