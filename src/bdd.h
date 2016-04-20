#ifndef _BDD_H_
#define _BDD_H_

#include "bdd_internal.h"

namespace bdd {
	class Bdd {
		public:
			Bdd();
			Bdd(Variable var);
			Bdd operator&(Bdd& r);
			Bdd operator|(Bdd& r);
			Bdd operator^(Bdd& r);

		private:
            Bdd(internal::Node* node);
            internal::Node* node;
	};
}

#endif
