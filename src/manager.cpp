#include <cassert>
#include <thread>

#include "bdd.h"

using namespace bdd_internal;

// The global manager object
Manager bdd_internal::manager;

static void thread_work(size_t index);

Manager::Manager() : thread_count(std::thread::hardware_concurrency()), threads(thread_count), notifiers(new ThreadWork[thread_count]) {

	add_nodes();

	for (size_t i = 0; i < thread_count; i++) {
		std::thread worker_thread(thread_work, i);
		worker_thread.detach();
	}
}

bool Manager::add_nodes() {
	constexpr size_t alloc_count = 64;

	Node (*new_nodes)[alloc_size] = reinterpret_cast<Node(*)[alloc_size]>(new Node[alloc_size * alloc_count]);
	if (new_nodes == nullptr) {
		return false;
	}

	for (size_t i = 0; i < alloc_count; i++) {
		main_nodes.push(&new_nodes[i]);
	}

	return true;
}

void Manager::thread_work(size_t index) {
	ThreadWork& work = manager.notifiers[index];
	// We'll hold this lock whenever we're not in the queue
	work.lock.lock();

	while (true) {
		// Mark that we're waiting for work
		work.result = nullptr;

		// Put ourselves in the queue and wait until someone brings us online
		manager.threads.enqueue(&work);
		work.ready.wait(work.lock, [&] { return work.result != nullptr; });


		//
		// Do work
		//


		// Store the result of the work
		work.result->lock.lock();
		work.result->data = nullptr; // Store data here
		work.result->lock.unlock();

		// Notify the user that it's ready
		work.result->ready.notify_one();
	}
}
