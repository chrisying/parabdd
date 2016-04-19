#include <cassert>
#include <limits>

#include "bdd.h"

namespace bdd {
	namespace internal {
		Node::Node() { }

		Node::Node(bdd::Variable root, bool complemented, Node* branch_true, Node* branch_false) : root(root), complemented(complemented), branch_true(branch_true), branch_false(branch_false) { }

		Node* Node::make(bdd::Variable root, Node* branch_true, Node* branch_false) {
			if (branch_true == branch_false) {
				return branch_false;
			}

            // Enforce canonicity (complement only on 1 edge)
            if ((!is_leaf(branch_false) && branch_false->complemented) || branch_false == true_node || branch_true == true_node) {
                Node* new_true = complement(branch_true);
                Node* new_false = complement(branch_false);
                Node node(root, true, new_true, new_false);
                return manager::nodes.lookupOrCreate(node);
            }

			Node node = Node(root, false, branch_true, branch_false);
			return manager::nodes.lookupOrCreate(node);
		}

		Node* Node::ITE(Node* A, Node* B, Node* C) {
            // Base cases
			if (A == true_node) { return B; }
			if (A == false_node) { return C; }
            if (B == true_node && C == false_node) { return A; }
            if (B == false_node && C == true_node) { return complement(A); }
            if (B == C) { return B; }

			// TODO: check if this ITE has been done before in cache

            // Normalization rules
            // TODO: order these rules efficiently?
            Node* result;

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
            } else if (B == true_node && C->root < A->root) {
                // ITE(A,1,C) -> ITE(C,1,A) if C < A
                result = ITE(C, true_node, A);
            } else if (B == false_node && C->root < A->root) {
                // ITE(A,0,C) -> ITE(!C,0,!A) if C < A
                result = ITE(complement(C), false_node, complement(A));
            } else if (C == true_node && B->root < A->root) {
                // ITE(A,B,1) -> ITE(!B,!A,1) if B < A
                result = ITE(complement(B), complement(A), true_node);
            } else if (C == false_node && B->root < A->root) {
                // ITE(A,B,0) -> ITE(B,A,0) if B < A
                result = ITE(B, A, false_node);
            } else if (equals_complement(B, C) && B->root < C->root) {
                // ITE(A,B,!B) -> ITE(B,A,!A)
                result = ITE(B, A, complement(A));
            } else if (A->complemented) {
                // ITE(A,B,C) -> ITE(!A,C,B) if A complemented
                result = ITE(complement(A), C, B);
            } else if (!is_leaf(B) && B->complemented) {
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

                Node* R_false = ITE(A_false, B_false, C_false);
                Node* R_true = ITE(A_true, B_true, C_true);

                result = make(x, R_true, R_false);
            }

			// TODO: put in cache

			return result;
		}

        bool Node::is_leaf(Node* node) {
            return node == true_node || node == false_node;
        }

        Variable Node::top_variable(Node* A, Node* B, Node* C) {
            auto var = [] (Node* x) {
                return (is_leaf(x) ? std::numeric_limits<Variable>::max() : x->root);
            };

            return std::min(var(A), std::min(var(B), var(C)));
        }

		Node* Node::evaluate_at(Node* node, bdd::Variable var, bool value) {
            // Variable is above this node, nothing changes
            if (is_leaf(node) || node->root > var) {
                return node;
            }

            // Variable is exactly this node, choose appropriate branch
			if (node->root == var) {
                // Logical XOR
                return value != node->complemented ? node->branch_true : node->branch_false;
			}

            // TODO: check cache now

            // Variable is below this node, recurse
			Node* new_node;
			if (value != node->complemented) { // Logical XOR
				new_node = make(var, evaluate_at(node->branch_true, var, value), evaluate_at(node->branch_false, var, value));
			} else {
				new_node = make(var, evaluate_at(node->branch_false, var, value), evaluate_at(node->branch_true, var, value));
			}

			// TODO: cache new_node

			return new_node;
		}

        // TODO: should this be inline?
        Node* Node::complement(Node* node) {
            if (node == true_node) {
                return false_node;
            } else if (node == false_node) {
                return true_node;
            }

            Node new_node = Node(node->root, !node->complemented, node->branch_true, node->branch_false);
            return manager::nodes.lookupOrCreate(new_node);
        }

        // Should only be used with nodes ALREADY in canonical form (outputs of make and ITE)
        // TODO: should I consider the case where complement is the same but A->true == B->false and vice versa
        bool Node::equals_complement(Node* A, Node* B) {
            if (is_leaf(A) && is_leaf(B) && A != B) {
                return true;
            } else if (is_leaf(A) || is_leaf(B)) {
                return false;
            }

            return (A->root == B->root && A->complemented != B->complemented && A->branch_true == B->branch_true && A->branch_false == B->branch_false);
        }

	}
}
