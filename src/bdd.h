typedef Variable uint32_t;
typedef BddIndex uint32_t;

type Query = ITE(A,B,C) | ... ;

// TODO: template based manager stuff
class Bdd {
	Variable root;
	BddIndex branch_true;
	BddIndex branch_false;
	uint32_t reference_count;
	bool negated_true;
	// TODO: how does it know which manager?

	Bdd(Variable var) {
	}

	Bdd(MK_form mk) {
	}

	operator+(Bdd& r) {
	}

	synthesis() {
	}
};

class BddManager {
	// TODO: how is ordering managed?
	Bdd nodes[]; // TODO: this can also be a free stack
	Table<Query, Bdd*> cache;
	Set<Bdd*> uniques;

	static const Bdd* trueBdd;
	static const Bdd* falseBdd;

	Bdd* new_node() {
		// Pull off the free stack and stuff
	}
};

// TODO: the hashset can do something like this
class Set<T> {
	lookup_insert(Bdd* x) {
		hash = h(*x);
		if (arr[hash]->vals == *x) {
			return;
		}
	}
};
