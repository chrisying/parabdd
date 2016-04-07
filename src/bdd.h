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

			Node();
			Node(bdd::Variable root, Node* branch_true, Node* branch_false);

		private:
			uint32_t reference_count;
	};

	struct WorkResult {
		std::unique_lock<std::mutex> lock;
		std::condition_variable ready;
		Node* data;

		void submit(Node* result) {
			lock.lock();
			data = result;
			lock.unlock();

			ready.notify_one();
		}
	};

	struct alignas(cache_width) ThreadWork {
		std::unique_lock<std::mutex> lock;
		std::condition_variable ready;
		uint32_t work;
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
			static constexpr Node* trueBdd = __builtin_constant_p((Node*) 1) ? (Node*) 1 : (Node*) 1;
			static constexpr Node* falseBdd = __builtin_constant_p((Node*) 2) ? (Node*) 2 : (Node*) 2;

			Manager();
			bool add_nodes();
			// Add a function to do work on behalf of threads

			static void thread_work();

		// TODO: how is ordering managed?
		private:
			static constexpr size_t alloc_size = 4096;
			std::mutex main_nodes_lock;
			std::stack<Node(*)[alloc_size]> main_nodes;

			std::unordered_map<Query, Node*> cache;
			Set<Node*> uniques;

			// TODO: thread-local node stacks
			// Threaded stuff here

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
