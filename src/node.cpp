#include <cassert>

#include "bdd.h"

namespace bdd {
	namespace internal {
		Node::Node() { }

		Node::Node(bdd::Variable root, Node* branch_true, Node* branch_false) : root(root), branch_true(branch_true), branch_false(branch_false) { }

		Node* Node::make(bdd::Variable root, Node* branch_true, Node* branch_false) {
			if (branch_true == branch_false) {
				return branch_false;
			}

			Node node = Node(root, branch_true, branch_false);
			return manager.uniques.lookupOrCreate(&node);
		}

		Node* Node::ITE(Node* A, Node* B, Node* C) {
			if (A == true_bdd) { return B; }
			if (A == false_bdd) { return C; }

			// TODO: check if this ITE has been done before in cache
            // TODO: normalization rules

			bdd::Variable x = std::max(std::max(A->root, B->root), C->root);
			Node* A_false = evaluate_at(A, x, false);
			Node* B_false = evaluate_at(B, x, false);
			Node* C_false = evaluate_at(C, x, false);
			Node* A_true = evaluate_at(A, x, true);
			Node* B_true = evaluate_at(B, x, true);
			Node* C_true = evaluate_at(C, x, true);

			Node* R_false = ITE(A_false, B_false, C_false);
			Node* R_true = ITE(A_true, B_true, C_true);

			Node* result = make(x, R_true, R_false);

			// TODO: put in cache

			return result;
		}

		Node* Node::evaluate_at(Node* node, bdd::Variable var, bool value) {
			// Evaluates the tree with var set to false and returns a Node*
			if (node->root == var) {
				if (value) {
					return node->branch_true;
				} else {
					return node->branch_false;
				}
			}

			Node* new_node;
			if (value) {
				new_node = make(var, evaluate_at(node->branch_true, var, value), evaluate_at(node->branch_false, var, value));
			} else {
				new_node = make(var, evaluate_at(node->branch_true, var, value), evaluate_at(node->branch_false, var, value));
			}

			// TODO: possibly cache this

			return new_node;
		}

	}
}
