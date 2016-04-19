#include <cassert>
#include <limits>

#include "bdd.h"

namespace bdd {
	Bdd::Bdd() : node(nullptr) { }

    Bdd::Bdd(Variable var) {
        node = internal::Node::make(var, internal::Node::true_node, internal::Node::false_node);
    }

    Bdd::Bdd(internal::Node* node) : node(node) { }

    Bdd Bdd::operator&(Bdd& r) {
        return Bdd(internal::Node::ITE(this->node, r.node, internal::Node::false_node));
    }

	Bdd Bdd::operator+(Bdd& r) {
        return internal::Node::ITE(this->node, internal::Node::true_node, r.node);
	}
}
