#include <cassert>

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
            if (branch_false->complemented) {
                Node new_true = complement(branch_true);
                Node new_false = complement(branch_false);
                Node node = Node(root, true, new_true, new_false);
                return manager::unique.lookupOrCreate(&node);
            }

			Node node = Node(root, false, branch_true, branch_false);
			return manager::uniques.lookupOrCreate(&node);
		}

		Node* Node::ITE(Node* A, Node* B, Node* C) {
            // Base cases
			if (A == true_bdd) { return B; }
			if (A == false_bdd) { return C; }
            if (B == true_bdd && C == false_bdd) { return A; }
            if (B == false_bdd && C == true_bdd) { return complement(A); }
            if (B == C) { return B; }

			// TODO: check if this ITE has been done before in cache

            // Normalization rules
            // TODO: order these rules efficiently?
            Node* result;

            if (A == B) {
                // ITE(A,A,C) -> ITE(A,1,C)
                result = ITE(A, true_bdd, C);
            } else if (equals_complement(A, B)) {
                // ITE(A,!A,C) -> ITE(A,0,C)
                result = ITE(A, false_bdd, C);
            } else if (A == C) {
                // ITE(A,B,A) -> ITE(A,B,0)
                result = ITE(A, B, false_bdd);
            } else if (equals_complement(A, C)) {
                // ITE(A,B,!A) -> ITE(A,B,1)
                result = ITE(A, B, true_bdd);
            } else if (B == true_bdd && C->root < A->root) {
                // ITE(A,1,C) -> ITE(C,1,A) if C < A
                result = ITE(C, true_bdd, A);
            } else if (B == false_bdd && C->root < A->root) {
                // ITE(A,0,C) -> ITE(!C,0,!A) if C < A
                result = ITE(complement(C), false_bdd, complement(A));
            } else if (C == true_bdd && B->root < A->root) {
                // ITE(A,B,1) -> ITE(!B,!A,1) if B < A
                result = ITE(complement(B), complement(A), true_bdd);
            } else if (C == false_bdd && B->root < A->root) {
                // ITE(A,B,0) -> ITE(B,A,0) if B < A
                result = ITE(B, A, false_bdd);
            } else if (equals_complement(B, C) && B->root < C->root) {
                // ITE(A,B,!B) -> ITE(B,A,!A)
                result = ITE(B, A, complement(A));
            } else if (A->complemented) {
                // ITE(A,B,C) -> ITE(!A,C,B) if A complemented
                result = ITE(complement(A), C, B);
            } else if (B->complemented) {
                // ITE(A,B,C) -> !ITE(A,!B,!C) if B complemented
                result = complement(ITE(A, complement(B), complement(C)));
            } else {
                // If no normalization applies
                bdd::Variable x = std::max(std::max(A->root, B->root), C->root);
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

		Node* Node::evaluate_at(Node* node, bdd::Variable var, bool value) {
            // Variable is above this node, nothing changes
            if (node->root > var) {
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
        Node* complement(Node* node) {
            return manager::uniques.lookupOrCreate(&Node(node->root, !node->complemented, node->branch_true, node->branch_false));
        }

        // Should only be used with nodes ALREADY in canonical form (outputs of make and ITE)
        // TODO: should I consider the case where complement is the same but A->true == B->false and vice versa
        bool equals_complement(Node* A, Node* B) {
            return (A->root == B->root && A->complemented != B->complemented && A->branch_true == B->branch_true && A->branch_false == B->branch_false);
        }

	}
}
