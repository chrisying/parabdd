#include <cassert>
#include <limits>
#include <iostream>

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
}
