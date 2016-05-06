#include <cassert>
#include <limits>
#include <iostream>
#include <cmath>
#include <atomic>
#include <set>

#include "bdd.h"

namespace bdd {
    static bool one_sat_helper(internal::Node* node, bool p, std::unordered_map<Variable, bool>& map);
    static long long count_sat_helper(internal::Node* node, int n, std::set<Variable>& vars);

    Bdd Bdd::bdd_true(internal::Node::true_node);
    Bdd Bdd::bdd_false(internal::Node::false_node);

	Bdd::Bdd() : node(nullptr) { }

    Bdd::Bdd(Variable var) {
        node = internal::Node::make(var, internal::Node::true_node, internal::Node::false_node);
    }

    Bdd::Bdd(internal::Node* node) : node(node) { }

    Bdd Bdd::operator!() {
        return Bdd(internal::Node::complement(this->node));
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
        return internal::Node::ITE(this->node, internal::Node::complement(r.node), r.node);
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
        return internal::Node::ITE(this->node, internal::Node::true_node, internal::Node::complement(r.node));
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
        if (one_sat_helper(this->node, !internal::Node::is_complemented(this->node), map)) {
            return map;
        } else {
            std::cout << "one_sat() returned with no solution" << std::endl;
            map.clear();
            return map;
        }
    }

    static bool one_sat_helper(internal::Node* node, bool p, std::unordered_map<Variable, bool>& map) {
        //std::cout << "Called one_sat with " << node << ", " << p << std::endl;
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
     * Count SAT
     **/

    long long Bdd::count_sat(std::set<Variable> vars) {
        int n = vars.size();
        long long pow2 = pow(2, n);
        long long count = count_sat_helper(this->node, n, vars);

        if (count == -1) {
            std::cout << "A variable in the BDD was not declared in vars" << std::endl;
            return -1;
        }

        if (!internal::Node::is_complemented(this->node)) {
            count = pow2 - count;
        }

        return count;
    }

    // TODO: I think there is a better way to implement this. The invariant
    // should be that count_sat_helper will return the exact number of SAT
    // assignments. We should do the negations at the base case and right
    // before adding to cache.
    static long long count_sat_helper(internal::Node* node, int n, std::set<Variable>& vars) {
        // TODO: handle overflow by using real doubles
        long long pow2 = pow(2, n);
        if (internal::Node::is_leaf(node)) {
            return pow2;
        }

        // TODO: check cache now

        internal::Node* dnode = internal::Node::pointer(node);
        if (!vars.count(dnode->root)) {
            return -1;
        }

        // TODO: this can be done in parallel
        long long countT = count_sat_helper(dnode->branch_true, n, vars);
        long long countF = count_sat_helper(dnode->branch_false, n, vars);

        if (countT == -1 || countF == -1) {
            return -1; // TODO: should -1 be cached?
        }

        if (internal::Node::is_complemented(dnode->branch_true)) {
            countT = pow2 - countT;
        }

        long long count = (countT + countF) / 2;

        // TODO: add to cache now

        return count;
    }
}
