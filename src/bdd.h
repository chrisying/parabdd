#ifndef _BDD_H_
#define _BDD_H_

#include <set>

#include "bdd_internal.h"

namespace bdd {
	class Bdd {
		public:
            // TODO: create static bddtrue, bddfalse
			Bdd();
			Bdd(Variable var);
            Bdd operator!();      // NOT
			Bdd operator&(Bdd r); // AND
            Bdd operator&=(Bdd r);
			Bdd operator|(Bdd r); // OR
            Bdd operator|=(Bdd r);
			Bdd operator^(Bdd r); // XOR
            Bdd operator^=(Bdd r);
            Bdd operator>(Bdd r); // IMPLIES
            Bdd operator>=(Bdd r);
            Bdd operator<(Bdd r); // REVERSE IMPLIES
            Bdd operator<=(Bdd r);

            static Bdd bdd_true(internal::Node::true_node);
            static Bdd bdd_false(internal::Node::false_node);

            std::unordered_map<Variable, bool> one_sat();
            int count_sat(std::set<Variable> vars);

		private:
            Bdd(internal::Node* node);
            internal::Node* node;
            bool one_sat_helper(internal::Node* node, bool p, std::unordered_map<Variable, bool>& map);
            int count_sat_helper(internal::Node* node, int n, std::set<Variable> vars);
	};
}

#endif
