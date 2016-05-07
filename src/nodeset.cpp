#include <cassert>

#include "nodeset.h"
#include "node.h"
#include "hash.h"

namespace bdd {
    namespace internal {
        static inline size_t hash(const Node& node) {
            return hash128(&node, sizeof(Node));
        }

        static void copy_node(const Node& src, Node& dest) {
            dest.root = src.root;
            dest.branch_true = src.branch_true;
            dest.branch_false = src.branch_false;
        }

        void NodeSet::init(size_t mem_usage) {
            _elems = mem_usage / sizeof(NodeSlot);
            assert(_elems < std::numeric_limits<int32_t>::max());
            table = (NodeSlot*) calloc(_elems, sizeof(NodeSlot)); // Much faster than running constructors

            assert(table != nullptr);
            table[0].exists = true;
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

        NodePtr NodeSet::lookupOrCreate(const Node& node) {
            uint32_t hashed = hash(node);

            for (uint32_t offset = 0; offset < _elems; offset++) {
                uint32_t index = (hashed + offset) % _elems;

                NodeSlot& current = table[index];
                LockProtector lock(current);

                if (!current.exists) {
                    copy_node(node, current.node);

                    // Maintain data structure
                    _count.fetch_add(1, std::memory_order_relaxed);
                    current.exists = true;

                    return index;
                }

                if (node.root == current.node.root &&
                    node.branch_true == current.node.branch_true &&
                    node.branch_false == current.node.branch_false) {

                    // TODO: increase reference count
                    // It's the node we're looking for!
                    return index;
                }
            }

            return 0;
        }
    }
}
