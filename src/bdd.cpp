#include <cassert>
#include <limits>
#include <iostream>
#include <cmath>

#include "bdd.h"

namespace bdd {
	Bdd::Bdd() : node(nullptr) { }

    Bdd::Bdd(Variable var) {
        node = internal::Node::make(var, internal::Node::true_node, internal::Node::false_node);
    }

    Bdd::Bdd(internal::Node* node) : node(node) { }

    Bdd Bdd::operator!() {
        return Bdd(internal::Node::complement(this->node));
    }

    Bdd Bdd::operator&(Bdd r) {
        return Bdd(internal::Node::ITE(this->node, r.node, internal::Node::false_node));
    }

	Bdd Bdd::operator|(Bdd r) {
        return internal::Node::ITE(this->node, internal::Node::true_node, r.node);
	}

	Bdd Bdd::operator^(Bdd r) {
        return internal::Node::ITE(this->node, internal::Node::complement(r.node), r.node);
	}

	Bdd Bdd::operator>(Bdd r) {
        return internal::Node::ITE(this->node, r.node, internal::Node::true_node);
	}

	Bdd Bdd::operator<(Bdd r) {
        return internal::Node::ITE(this->node, internal::Node::true_node, internal::Node::complement(r.node));
	}

    /**
     * One SAT
     **/

    std::unordered_map<Variable, bool> Bdd::one_sat() {
        std::unordered_map<Variable, bool> map;
        one_sat_helper(this->node, !internal::Node::is_complemented(this->node), map);
        return map;
    }

    bool Bdd::one_sat_helper(internal::Node* node, bool p, std::unordered_map<Variable, bool>& map) {
        std::cout << "Called one_sat with " << node << ", " << p << std::endl;
        if (internal::Node::is_leaf(node)) {
            return !p;
        }

        internal::Node* dnode = internal::Node::pointer(node);

        map[dnode->root] = false;
        if (one_sat_helper(dnode->branch_false, p, map)) {
            return true;
        }

        map[dnode->root] = true;
        if (internal::Node::is_complemented(dnode->branch_true)) {
            p = !p;
        }

        return one_sat_helper(dnode->branch_true, p, map);
    }

    /**
     * One SAT
     **/

    int Bdd::count_sat() {
        // TODO: write this after we augment number of variables
        // ISSUE: if we don't use a variable in a BDD, even if we want to
        // count it (ex: T/F doesn't matter but it should still be counted
        // as a var), it won't be counted.
        return 0;
    }

    // TODO: I think there is a better way to implement this. The invariant
    // should be that count_sat_helper will return the exact number of SAT
    // assignments. We should do the negations at the base case and right
    // before adding to cache.
    int Bdd::count_sat_helper(internal::Node* node, int n) {
        // TODO: handle overflow
        if (internal::Node::is_leaf(node)) {
            return pow(2, n);
        }

        // TODO: check cache now

        internal::Node* dnode = internal::Node::pointer(node);
        int countT = count_sat_helper(dnode->branch_true, n);
        int countF = count_sat_helper(dnode->branch_false, n);

        if (internal::Node::is_complemented(dnode->branch_true)) {
            countF = pow(2, n) - countF;
        }

        int count = (countT + countF) / 2;

        // TODO: add to cache now

        return count;
    }
}
