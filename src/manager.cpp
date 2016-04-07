#include <cassert>
#include <thread>
#include <algorithm>

#include "bdd.h"

using namespace bdd_internal;

// The global manager object
Manager bdd_internal::manager;

static void thread_work(size_t index);

Manager::Manager() : threads(std::thread::hardware_concurrency()) {

	add_nodes();

	for (size_t i = std::thread::hardware_concurrency(); i > 0; i--) {
		std::thread worker_thread(thread_work);
		worker_thread.detach();
	}
}

bool Manager::add_nodes() {
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

Node* Manager::make(bdd::Variable root, Node* branch_true, Node* branch_false) {
    if (branch_true == branch_false) {
        return branch_false;
    }

    Node node = Node(root, branch_true, branch_false);
    return manager.uniques.lookupOrCreate(&node);
}

Node* Manager::ITE(Node* A, Node* B, Node* C) {
    if (A == true_bdd) { return B; }
    if (A == false_bdd) { return C; }

    // TODO: check if this ITE has been done before in cache

    bdd::Variable x = std::max(std::max(A->root, B->root), C->root);
    Node* A_false = evaluate_at(A, x, false);
    Node* B_false = evaluate_at(B, x, false);
    Node* C_false = evaluate_at(C, x, false);
    Node* A_true = evaluate_at(A, x, true);
    Node* B_true = evaluate_at(B, x, true);
    Node* C_true = evaluate_at(C, x, true);

    Node* R_false = ITE(A_false, B_false, C_false);
    Node* R_true = ITE(A_true, B_true, C_true);

    Node* result = make(x, R_true, R_false);

    // TODO: put in cache

    return result;
}

Node* Manager::evaluate_at(Node* node, bdd::Variable var, bool value) {
    // Evaluates the tree with var set to false and returns a Node*
    if (node->root == var) {
        if (value) {
            return node->branch_true;
        } else {
            return node->branch_false;
        }
    }

    Node* new_node;
    if (value) {
        new_node = make(var, evaluate_at(node->branch_true, var, value), evaluate_at(node->branch_false, var, value));
    } else {
        new_node = make(var, evaluate_at(node->branch_true, var, value), evaluate_at(node->branch_false, var, value));
    }

    // TODO: possibly cache this

    return new_node;
}

void Manager::thread_work() {
	thread_local std::stack<Node*> free_nodes;
	thread_local ThreadWork work;

	// We'll hold this lock whenever we're not in the queue
	std::lock_guard<std::unique_lock<std::mutex>> lock(work.lock);

	while (true) {
		// Mark that we're waiting for work
		work.result = nullptr;

		// Put ourselves in the queue and wait until someone brings us online
		manager.threads.enqueue(&work);
		work.ready.wait(work.lock, [&] { return work.result != nullptr; });


		//
		// Do work
		//


		// Store the result of the work and notify the user that it's ready
		work.result->submit(nullptr);
	}
}
