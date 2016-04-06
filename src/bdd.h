#ifndef _BDD_H_
#define _BDD_H_

#include <cstdint>
#include <unordered_map>
#include <set>
#include <stack>

#include "set.h"

namespace bdd {
	typedef uint32_t Variable;
}

namespace bdd_internal {
	typedef int Query; // Temporary hack

	class Node {
		public:
			// Unique
			bdd::Variable root;
			Node* branch_true;
			Node* branch_false;
			bool negated_true;  // Is this unique?

			Node();
			Node(bdd::Variable root, Node* branch_true, Node* branch_false);

		private:
			uint32_t reference_count;
	};

	class Manager {
		public:
			static const Node* trueBdd;
			static const Node* falseBdd;

			Manager();
			bool add_nodes();

		// TODO: how is ordering managed?
		private:
			static constexpr size_t alloc_size = 4096;
			std::stack<Node(*)[alloc_size]> main_nodes;
			// TODO: thread-local node stacks

			std::unordered_map<Query, Node*> cache;
			Set<Node*> uniques;

			size_t thread_count;
			// WorkStack
	};

	extern Manager manager;
}

namespace bdd {
	using bdd_internal::Node;

	class Bdd {
		public:
			Bdd();
			Bdd operator+(Bdd& r);
			Bdd operator^(Bdd& r);
			Bdd operator&(Bdd& r);

		private:
			Node* bdd;
	};
}

#endif
