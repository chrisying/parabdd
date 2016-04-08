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

	namespace internal {
		typedef int Query; // Temporary hack
		typedef int Work; // Temporary hack

		// Cache line width (bytes) on x86
		constexpr size_t cache_width = 64;

		// Our internal BDD representation
		class Node {
			public:
				// Uniquely identifying BDDs in canonical form
				bdd::Variable root;
				// TODO: complement on a node (canonicity self-enforced, not by type)
				Node* branch_true;
				Node* branch_false;

				Node();
				// Creates node on stack, should be used in MK to get heap pointer
				Node(bdd::Variable root, Node* branch_true, Node* branch_false);

				static Node* make_node(bdd::Variable root, Node* branch_true, Node* branch_false);
				static Node* ITE(Node* A, Node* B, Node* C);
				static Node* evaluate_at(Node* node, bdd::Variable var, bool value);
				static Node* complement(Node* node);

			private:
				// A reference count for freeing temporary BDDs
				uint32_t reference_count;
		};

		// Stores the result of a future computation, allowing the creator to
		// wait for it to be completed.
		struct WorkResult {
			std::mutex lock;
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
			std::mutex lock;
			std::condition_variable ready;
			Work work;
			WorkResult* result;

			void submit(Work& new_work, WorkResult* result_loc) {
				lock.lock();
				work = new_work;
				result = result_loc;
				lock.unlock();

				ready.notify_one();
			}
		};

		namespace manager {
			// SeemsGood -- Documented GCC/Clang builtins hack
			constexpr size_t alloc_size = 4096;
			constexpr Node* true_bdd = __builtin_constant_p((Node*) 1) ? (Node*) 1 : (Node*) 1;
			constexpr Node* false_bdd = __builtin_constant_p((Node*) 2) ? (Node*) 2 : (Node*) 2;

			bool add_nodes();


			// TODO: Add a function to do work on behalf of threads

			// TODO: how is ordering managed?
			extern std::mutex main_nodes_lock;
			extern std::stack<Node(*)[alloc_size]> main_nodes;

			extern std::unordered_map<Query, Node*> cache;
			extern Set<Node*> uniques;

			extern Queue<ThreadWork*> threads;
		}
	}

	class Bdd {
		public:
			Bdd();
			Bdd(Variable var);
			Bdd(internal::Node* node);
			Bdd operator&(Bdd& r);
			Bdd operator+(Bdd& r);
			Bdd operator^(Bdd& r);

		private:
			internal::Node* bdd;
	};
}

#endif
