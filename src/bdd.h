#ifndef _BDD_H_
#define _BDD_H_

#include <cstdint>
#include <unordered_map>
#include <set>
#include <stack>
#include <thread>
#include <condition_variable>

#include "set.h"
#include "queue.h"

namespace bdd {
	typedef uint32_t Variable;
}

namespace bdd_internal {
	typedef int Query; // Temporary hack

	// Cache line width (bytes) on x86
	constexpr size_t cache_width = 64;

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

	typedef struct WorkResult {
		std::unique_lock<std::mutex> lock;
		std::condition_variable ready;
		Node* data;
	} WorkResult;

	typedef struct alignas(cache_width) ThreadWork {
		std::unique_lock<std::mutex> lock;
		std::condition_variable ready;
		uint32_t work;
		WorkResult* result;
	} ThreadWork;

	class Manager {
		public:
			static constexpr Node* trueBdd = reinterpret_cast<Node*>(1);
			static constexpr Node* falseBdd = reinterpret_cast<Node*>(2);

			Manager();
			bool add_nodes();
			// Add a function to do work on behalf of threads

			static void thread_work(size_t index);

		// TODO: how is ordering managed?
		private:
			static constexpr size_t alloc_size = 4096;
			std::stack<Node(*)[alloc_size]> main_nodes;
			std::unordered_map<Query, Node*> cache;
			Set<Node*> uniques;

			// TODO: thread-local node stacks
			// Threaded stuff here

			size_t thread_count;
			ThreadWork* notifiers;
			Queue<ThreadWork*> threads;
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
