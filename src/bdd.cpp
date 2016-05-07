#include <cassert>
#include <limits>
#include <iostream>
#include <cmath>
#include <atomic>
#include <set>

#include "bdd.h"

namespace bdd {
    // Inverts the lowest order bit
    static inline internal::Node* complement(internal::Node* node) {
        return reinterpret_cast<internal::Node*>(((uintptr_t) node) ^ 0x1);
    }

    static inline bool is_complemented(internal::Node* node) {
        return (((uintptr_t) node) & 0x1) == 0x1;
    }

    static inline bool is_leaf(internal::Node* node) {
        return node == internal::Node::true_node || node == internal::Node::false_node;
    }

    // Sets lowest order bit to 0
    static inline internal::Node* pointer(internal::Node* node) {
        return reinterpret_cast<internal::Node*>(((uintptr_t) node) & ((uintptr_t) ~0x1LL));
    }

    static bool one_sat_helper(internal::Node* node, bool p, std::unordered_map<Variable, bool>& map);
    static double count_sat_helper(internal::Node* node, int n, std::set<Variable>& vars);

    Bdd Bdd::bdd_true(internal::Node::true_node);
    Bdd Bdd::bdd_false(internal::Node::false_node);

	Bdd::Bdd() : node(nullptr) { }

    Bdd::Bdd(Variable var) {
        node = internal::Node::make(var, internal::Node::true_node, internal::Node::false_node);
    }

    Bdd::Bdd(internal::Node* node) : node(node) { }

    Bdd Bdd::operator!() {
        return Bdd(complement(this->node));
    }

    Bdd Bdd::operator&(Bdd r) {
        //Bdd temp = Bdd(internal::Node::ITE(this->node, r.node, internal::Node::false_node));
        //internal::Node::print_node(temp.node, 0);
        return Bdd(internal::Node::ITE(this->node, r.node, internal::Node::false_node));
    }

    Bdd Bdd::operator&=(Bdd r) {
        *this = *this & r;
        return *this;
    }

	Bdd Bdd::operator|(Bdd r) {
        return internal::Node::ITE(this->node, internal::Node::true_node, r.node);
	}

    Bdd Bdd::operator|=(Bdd r) {
        *this = *this | r;
        return *this;
    }

	Bdd Bdd::operator^(Bdd r) {
        return internal::Node::ITE(this->node, complement(r.node), r.node);
	}

    Bdd Bdd::operator^=(Bdd r) {
        *this = *this ^ r;
        return *this;
    }

	Bdd Bdd::operator>(Bdd r) {
        return internal::Node::ITE(this->node, r.node, internal::Node::true_node);
	}

    Bdd Bdd::operator>=(Bdd r) {
        *this = *this > r;
        return *this;
    }

	Bdd Bdd::operator<(Bdd r) {
        return internal::Node::ITE(this->node, internal::Node::true_node, complement(r.node));
	}

    Bdd Bdd::operator<=(Bdd r) {
        *this = *this < r;
        return *this;
    }

    void Bdd::print(std::string title) {
        internal::Node::print(this->node, title);
    }

    /**
     * One SAT
     **/

    std::unordered_map<Variable, bool> Bdd::one_sat() {
        std::unordered_map<Variable, bool> map;
        if (one_sat_helper(this->node, !is_complemented(this->node), map)) {
            return map;
        } else {
            std::cout << "one_sat() returned with no solution" << std::endl;
            map.clear();
            return map;
        }
    }

    static bool one_sat_helper(internal::Node* node, bool p, std::unordered_map<Variable, bool>& map) {
        //std::cout << "Called one_sat with " << node << ", " << p << std::endl;
        if (is_leaf(node)) {
            return !p;
        }

        internal::Node* dnode = pointer(node);

        map[dnode->root] = false;
        if (one_sat_helper(dnode->branch_false, p, map)) {
            return true;
        }

        map[dnode->root] = true;
        if (is_complemented(dnode->branch_true)) {
            p = !p;
        }

        return one_sat_helper(dnode->branch_true, p, map);
    }

    /**
     * Count SAT
     **/

    double Bdd::count_sat(std::set<Variable> vars) {
        int n = vars.size();
        double pow2 = pow(2, n);
        double count = count_sat_helper(this->node, n, vars);

        //if (count == -1) {
        //    std::cout << "A variable in the BDD was not declared in vars" << std::endl;
        //    return -1;
        //}

        if (!is_complemented(this->node)) {
            count = pow2 - count;
        }

        return count;
    }

    // TODO: I think there is a better way to implement this. The invariant
    // should be that count_sat_helper will return the exact number of SAT
    // assignments. We should do the negations at the base case and right
    // before adding to cache.
    static double count_sat_helper(internal::Node* node, int n, std::set<Variable>& vars) {
        // TODO: handle overflow by using real doubles
        double pow2 = pow(2, n);
        if (is_leaf(node)) {
            return pow2;
        }

        // TODO: check cache now

        internal::Node* dnode = pointer(node);
        if (vars.count(dnode->root) == 0) {
            std::cout << "Undeclared variable: " << dnode->root << std::endl;
            assert(false);
        }

        // TODO: this can be done in parallel
        double countT = count_sat_helper(dnode->branch_true, n, vars);
        double countF = count_sat_helper(dnode->branch_false, n, vars);

        if (is_complemented(dnode->branch_true)) {
            countT = pow2 - countT;
        }

        double count = countT + (countF - countT) / 2;
        //assert(count >= 0);

        // TODO: add to cache now

        return count;
    }
}
