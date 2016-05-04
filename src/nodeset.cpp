#include "nodeset.h"
#include "node.h"

namespace bdd {
    namespace internal {
        static uintptr_t hash64(uintptr_t key) {
            key = (~key) + (key << 21); // key = (key << 21) - key - 1;
            key = key ^ (key >> 24);
            key = (key + (key << 3)) + (key << 8); // key * 265
            key = key ^ (key >> 14);
            key = (key + (key << 2)) + (key << 4); // key * 21
            key = key ^ (key >> 28);
            key = key + (key << 31);
            return key;
        }

        static size_t hash(const Node& node) {
            uintptr_t var_hash = hash64(static_cast<uintptr_t>(node.root));
            uintptr_t true_hash = hash64(reinterpret_cast<uintptr_t>(node.branch_true));
            uintptr_t false_hash = hash64(reinterpret_cast<uintptr_t>(node.branch_false));

            return var_hash ^ true_hash ^ false_hash;
        }

        static void copy_node(const Node& src, Node& dest) {
            dest.root = src.root;
            dest.branch_true = src.branch_true;
            dest.branch_false = src.branch_false;
            dest.reference_count = 1; // Release the modification lock
        }

        bool NodeSet::init(size_t mem_usage) {
            _elems = mem_usage / sizeof(NodeSlot);
            _table = new NodeSlot[_elems];

            return (_table != nullptr);
        }

        // A lock guard around nodes
        struct LockProtector {
            public:
                LockProtector(NodeSlot& slot) : _slot(slot) {
                    do {
                        while (_slot.locked);
                    } while (_slot.locked.exchange(true));
                }
                ~LockProtector() {
                    _slot.locked = false;
                }

            private:
                NodeSlot& _slot;
        };

        Node* NodeSet::lookupOrCreate(const Node& node) {
            size_t hashed = hash(node);

            bool retry_freed = false;
            bool freed_seen = false;

            for (size_t offset = 0; offset < _elems; offset++) {
                size_t index = (hashed + offset) % _elems;

                LockProtector lock(_table[index]);
                Node& current = _table[index].node;

                if (current.reference_count == Node::unused) {
                    // It's an unused node - we should take it if there are no free
                    // nodes to go back to.
                    if (freed_seen && !retry_freed) {
                        retry_freed = true;

                        offset = std::numeric_limits<size_t>::max();
                        continue;
                    }
                    else {
                        copy_node(node, current);
                        return &current;
                    }

                    break;
                }
                else if (current.reference_count == Node::freed) {
                    // We're looking for a freed node
                    if (retry_freed) {
                        copy_node(node, current);
                        return &current;
                    }

                    // Freed node - come back to it if necessary
                    freed_seen = true;
                }
                else if (node.root == current.root && node.branch_true == current.branch_true && node.branch_false == current.branch_false) {
                    // It's the node we're looking for!  Increase its reference count.
                    current.reference_count++;
                    return &current;
                }
            }

            return nullptr;
        }
    }
}
