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
        }

        bool NodeSet::init(size_t mem_usage) {
            _elems = mem_usage / sizeof(NodeSlot);
            _table = (NodeSlot*) calloc(_elems, sizeof(NodeSlot)); // Much faster than running constructors

            return (_table != nullptr);
        }

        // A lock guard around nodes
        struct LockProtector {
            public:
                LockProtector(NodeSlot& slot) : _slot(slot) {
                    while (_slot.locked.test_and_set(std::memory_order_acquire));
                }
                ~LockProtector() {
                    _slot.locked.clear(std::memory_order_release);
                }

            private:
                NodeSlot& _slot;
        };

        Node* NodeSet::lookupOrCreate(const Node& node) {
            size_t hashed = hash(node);

            for (size_t offset = 0; offset < _elems; offset++) {
                size_t index = (hashed + offset) % _elems;

                NodeSlot& current = _table[index];
                LockProtector lock(current);

                if (!current.exists) {
                    copy_node(node, current.node);

                    // Maintain data structure
                    _count.fetch_add(1, std::memory_order_relaxed);
                    current.exists = true;

                    return &current.node;
                }

                if (node.root == current.node.root &&
                    node.branch_true == current.node.branch_true &&
                    node.branch_false == current.node.branch_false) {

                    // TODO: increase reference count
                    // It's the node we're looking for!
                    return &current.node;
                }
            }

            return nullptr;
        }
    }
}
