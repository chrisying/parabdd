#ifndef _BDD_H_
#define _BDD_H_

#include <set>
#include <string>

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

            static Bdd bdd_true;
            static Bdd bdd_false;

            std::unordered_map<Variable, bool> one_sat();
            double count_sat(std::set<Variable> vars);
            void print(std::string title);

		private:
            Bdd(internal::Node* node);
            internal::Node* node;
	};
}

#endif
