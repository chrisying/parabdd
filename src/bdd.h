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
	// The type of a BDD variable
	typedef uint32_t Variable;
}

namespace bdd_internal {
	typedef int Query; // Temporary hack
	typedef int Work; // Temporary hack

	// Cache line width (bytes) on x86
	constexpr size_t cache_width = 64;

	// Our internal BDD representation
	// TODO: use complemented edges
	class Node {
		public:
			// Uniquely identifying BDDs in canonical form
			bdd::Variable root;
			Node* branch_true;
			Node* branch_false;

			Node();
            // Creates node on stack, should be used in MK to get heap pointer
			Node(bdd::Variable root, Node* branch_true, Node* branch_false);

		private:
			// A reference count for freeing temporary BDDs
			uint32_t reference_count;
	};

	// Stores the result of a future computation, allowing the creator to
	// wait for it to be completed.
	struct WorkResult {
		std::unique_lock<std::mutex> lock;
		std::condition_variable ready;
		Node* data;

		// Submit a result, waking the creator if necessary
		void submit(Node* result) {
			lock.lock();
			data = result;
			lock.unlock();

			ready.notify_one();
		}
	};

	// Stores the work area for a thread, allowing users to submit a job
	// and provide an area to write the result to.
	// The owning thread will sleep until notified by `submit` that a job
	// is available.
	struct ThreadWork {
		std::unique_lock<std::mutex> lock;
		std::condition_variable ready;
		Work work;
		WorkResult* result;

		void submit(uint32_t& new_work, WorkResult* result_loc) {
			lock.lock();
			work = new_work;
			result = result_loc;
			lock.unlock();

			ready.notify_one();
		}
	};

	class Manager {
		public:
			// SeemsGood -- Documented GCC/Clang builtins hack
			static constexpr Node* true_bdd = __builtin_constant_p((Node*) 1) ? (Node*) 1 : (Node*) 1;
			static constexpr Node* false_bdd = __builtin_constant_p((Node*) 2) ? (Node*) 2 : (Node*) 2;

			Manager();
			bool add_nodes();

            // TODO: MK, ITE, and other related functions should be somewhere else probably
            // Returns a pointer on the heap
            static Node* make(bdd::Variable root, Node* branch_true, Node* branch_false);
            static Node* ITE(Node* A, Node* B, Node* C);

			// Add a function to do work on behalf of threads

			static void thread_work();

		// TODO: how is ordering managed?
		private:
			static constexpr size_t alloc_size = 4096;
			std::mutex main_nodes_lock;
			std::stack<Node(*)[alloc_size]> main_nodes;

			std::unordered_map<Query, Node*> cache;
			Set<Node*> uniques;

			// TODO: this probably shouldn't be here
            static Node* evaluate_at(Node* node, bdd::Variable var, bool value);

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
