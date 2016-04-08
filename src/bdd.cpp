#include <cassert>
#include <limits>

#include "bdd.h"

namespace bdd {
	Bdd::Bdd() : node(nullptr) { }

    Bdd::Bdd(Variable var) {
        node = internal::Node::make(var, internal::Node::true_bdd, internal::Node::false_bdd);
    }

    Bdd::Bdd(internal::Node* node) : node(node) { }

    Bdd Bdd::operator&(Bdd& r) {
        return Bdd(internal::Node::ITE(this->node, r.node, internal::Node::false_bdd));
    }

	Bdd Bdd::operator+(Bdd& r) {
        return internal::Node::ITE(this->node, internal::Node::true_bdd, r.node);
	}
}
