#include <limits>
#include <cstdint>

#include "bdd.h"

namespace bdd {
	namespace internal {

		Node::Node() { }

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

                Node* R_false = ITE(A_false, B_false, C_false);
                Node* R_true = ITE(A_true, B_true, C_true);

                result = make(x, R_true, R_false);
            }

			// TODO: put in cache

			return result;
		}

		Node* Node::evaluate_at(Node* node, Variable var, bool value) {
            // Variable is above this node, nothing changes
            if (is_leaf(node) || pointer(node)->root > var) {
                return node;
            }

            // Variable is exactly this node, choose appropriate branch
			if (pointer(node)->root == var) {
                // Logical XOR
                return value != is_complemented(node) ? pointer(node)->branch_true : pointer(node)->branch_false;
			}

            // TODO: check cache now

            // Variable is below this node, recurse
            // TODO: are these two different methods equivalent?
            // Option 1: check complement and do something different depending on whether it is complemented or not
			//Node* new_node;
			//if (value != is_complemented(node)) { // Logical XOR
			//	new_node = make(var, evaluate_at(deref(node).branch_true, var, value), evaluate_at(deref(node).branch_false, var, value));
			//} else {
			//	new_node = make(var, evaluate_at(deref(node).branch_false, var, value), evaluate_at(deref(node).branch_true, var, value));
			//}
            // Option 2: evaluate recursive case and complement it after
            Node* new_node = make(var, evaluate_at(pointer(node)->branch_true, var, value), evaluate_at(pointer(node)->branch_false, var, value));
            new_node = is_complemented(node) ? complement(new_node) : new_node;

			// TODO: cache new_node

			return new_node;
		}

        // Inverts the lowest order bit
        Node* Node::complement(Node* node) {
            return reinterpret_cast<Node*>(((uint64_t) node) ^ 0x1);
        }

        bool Node::is_complemented(Node* node) {
            return (((uint64_t) node) & 0x1) == 0x1;
        }

        // True iff A and B are the same except the lowest order bit
        bool Node::equals_complement(Node* A, Node* B) {
            return A == complement(B);
        }

        bool Node::is_leaf(Node* node) {
            return node == Node::true_node || node == Node::false_node;
        }

        Variable Node::top_variable(Node* A, Node* B, Node* C) {
            auto var = [] (Node* x) {
                return (is_leaf(x) ? std::numeric_limits<Variable>::max() : pointer(x)->root);
            };

            return std::min(var(A), std::min(var(B), var(C)));
        }

        // Sets lowest order bit to 0
        Node* Node::pointer(Node* node) {
            return reinterpret_cast<Node*>(((uint64_t) node) & ((uint64_t) ~0x1));
        }

        static void Node::print_node(Node* node) {
            Node* dnode = pointer(node);
            std::cout << "[Variable: " << dnode->root << ", Comp: ";
            if (is_complemented(this->node)) {
                std::cout << "yes] {" << std::endl;
            } else {
                std::cout << "no] {" << std:endl;
            }

            std::cout << "True branch (Variable " << dnode->root << "): ";
            print_node(dnode->branch_true);
            std::cout << "False branch (Variable " << dnode->root << "): ";
            print_node(dnode->branch_false);

            std::cout << "} #end Variable " << dnode->root << std::endl;
        }
	}
}
