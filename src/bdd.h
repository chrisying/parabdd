#ifndef _BDD_H_
#define _BDD_H_

#include <unordered_map>

#include "bdd_internal.h"

namespace bdd {
	class Bdd {
		public:
			Bdd();
			Bdd(Variable var);
            Bdd operator!();       // NOT
			Bdd operator&(Bdd& r); // AND
			Bdd operator|(Bdd& r); // OR
			Bdd operator^(Bdd& r); // XOR
            Bdd operator>(Bdd& r); // IMPLIES
            Bdd operator<(Bdd& r); // REVERSE IMPLIES

            std::unordered_map<Variable, bool> one_sat();

		private:
            Bdd(internal::Node* node);
            internal::Node* node;
            bool one_sat_helper(internal::Node* node, bool p, std::unordered_map<Variable, bool> map);
	};
}

#endif
