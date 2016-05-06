#include <cassert>
#include <limits>
#include <iostream>
#include <cmath>
#include <atomic>
#include <tuple>

#include "bdd.h"

namespace bdd {
    static bool one_sat_helper(internal::Node* node, bool p, std::unordered_map<Variable, bool>& map);
    static std::tuple<double, int> count_sat_helper(internal::Node* node, bool parity);

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

    double Bdd::count_sat() {
        bool parity = true;
        if (internal::Node::is_complemented(this->node)) {
            parity = false;
        }

        std::tuple<double, int> pair = count_sat_helper(this->node, parity);
        return std::get<0>(pair);
    }

    std::tuple<double, int> count_sat_helper(internal::Node* node, bool parity) {
        if (internal::Node::is_leaf(node) && !parity) {
            return std::make_tuple(1, 0);
        }

        internal::Node* dnode = internal::Node::pointer(node);
        std::tuple<double, int> p1 = count_sat_helper(dnode->branch_false, parity);
        std::tuple<double, int> p2;
        if (internal::Node::is_complemented(dnode->branch_true)) {
            p2 = count_sat_helper(dnode->branch_true, !parity);
        } else {
            p2 = count_sat_helper(dnode->branch_true, parity);
        }

        double count = 0;
        //count += pow(2, (number of unique nodes in this) - (number of unique nodes in this->branch_false) * std::get<0>(p1);
        //count += pow(2, (number of unique nodes in this) - (number of unique nodes in this->branch_true) * std::get<0>(p2);
        
        return std::make_tuple(0, 0); // TODO: this is wrong
    }

    //unsigned long long Bdd::count_sat(std::set<Variable> vars) {
    //    int n = vars.size();
    //    double pow2 = pow(2, n);
    //    double count = count_sat_helper(this->node, n, vars);

    //    //if (count == -1) {
    //    //    std::cout << "A variable in the BDD was not declared in vars" << std::endl;
    //    //    return -1;
    //    //}

    //    if (!internal::Node::is_complemented(this->node)) {
    //        count = pow2 - count;
    //    }
    //    std::cout << "before final return " << count << std::endl;

    //    return static_cast<unsigned long long>(count);
    //}

    //// TODO: I think there is a better way to implement this. The invariant
    //// should be that count_sat_helper will return the exact number of SAT
    //// assignments. We should do the negations at the base case and right
    //// before adding to cache.
    //static double count_sat_helper(internal::Node* node, double n, std::set<Variable>& vars) {
    //    // TODO: handle overflow by using real doubles
    //    double pow2 = pow(2, n);
    //    if (internal::Node::is_leaf(node)) {
    //        return pow2;
    //    }

    //    // TODO: check cache now

    //    internal::Node* dnode = internal::Node::pointer(node);
    //    // Fails if there is var in BDD not declared in vars
    //    assert(vars.count(dnode->root) != 0);

    //    // TODO: this can be done in parallel
    //    double countT = count_sat_helper(dnode->branch_true, n, vars);
    //    double countF = count_sat_helper(dnode->branch_false, n, vars);
    //    std::cout << "countT: " << countT << " countF: " << countF << std::endl;

    //    if (internal::Node::is_complemented(dnode->branch_true)) {
    //        countT = pow2 - countT;
    //    }

    //    double count = (countT + countF) / 2;
    //    std::cout << "returning " << count << std::endl;

    //    // TODO: add to cache now

    //    return count;
    //}
}
