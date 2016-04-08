#include <cassert>
#include <limits>

#include "bdd.h"

namespace bdd {
	Bdd::Bdd() : bdd(nullptr) { }

    Bdd::Bdd(Variable var) {
        bdd = bdd::Node::make_node(var, bdd::Node::true_bdd, bdd::Node::false_bdd);
    }

    Bdd::Bdd(bdd::Node* node) {
        bdd = node;
    }

    Bdd Bdd::operator&(Bdd& r) {
        return bdd::Node::ITE(this->bdd, r.bdd, bdd::Node::false_bdd);
    }

	Bdd Bdd::operator+(Bdd& r) {
        return bdd::Node::ITE(this->bdd, bdd::Node::true_bdd, r.bdd);
	}
}
