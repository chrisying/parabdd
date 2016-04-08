#include <cassert>
#include <thread>
#include <algorithm>

#include "bdd.h"

namespace bdd_internal {
	static void thread_work();

	namespace manager {
		std::mutex main_nodes_lock;
		std::stack<Node(*)[alloc_size]> main_nodes;

		std::unordered_map<Query, Node*> cache;
		Set<Node*> uniques;

		Queue<ThreadWork*> threads(std::thread::hardware_concurrency());

		// A dummy constructor allowing us to intiailize the manager state
		// TODO: is there a better way?
		struct ManagerConstructor {
			ManagerConstructor() {
				add_nodes();

				for (size_t i = std::thread::hardware_concurrency(); i > 0; i--) {
					std::thread worker_thread(thread_work);
					worker_thread.detach();
				}
			}
		} constructor;

		bool add_nodes() {
			constexpr size_t alloc_count = 64;

			Node (*new_nodes)[alloc_size] = reinterpret_cast<Node(*)[alloc_size]>(new Node[alloc_size * alloc_count]);
			if (new_nodes == nullptr) {
				return false;
			}

			main_nodes_lock.lock();

			for (size_t i = 0; i < alloc_count; i++) {
				main_nodes.push(&new_nodes[i]);
			}

			main_nodes_lock.unlock();

			return true;
		}
	}

	void thread_work() {
		thread_local std::stack<Node*> free_nodes;
		thread_local ThreadWork work;

		// We'll hold this lock whenever we're not in the queue
		std::unique_lock<std::mutex> lock(work.lock);

		while (true) {
			// Mark that we're waiting for work
			work.result = nullptr;

			// Put ourselves in the queue and wait until someone brings us online
			manager::threads.enqueue(&work);
			work.ready.wait(lock, [&] { return work.result != nullptr; });


			//
			// Do work
			//


			// Store the result of the work and notify the user that it's ready
			work.result->submit(nullptr);
		}
	}
}
