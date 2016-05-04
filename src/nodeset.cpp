#include <algorithm>

#include "nodeset.h"
#include "node.h"

namespace bdd {
    namespace internal {
        bool NodeSet::init(size_t mem_usage) {
            size_t num_nodes = mem_usage / sizeof(Node);
            printf("%lu nodes\n", num_nodes);

            table = (Node*) calloc(num_nodes, sizeof(Node));
            return true;
        }

        Node* NodeSet::lookupOrCreate(const Node& node) {
            std::list<Node*>::iterator it = std::find_if(_set.begin(), _set.end(), [&](Node* elem) {
                return (node.root == elem->root && node.branch_true == elem->branch_true && node.branch_false == elem->branch_false);
            });

            if (it == _set.end()) { // No match found, allocate and insert
                Node* new_node = new Node(node.root, node.branch_true, node.branch_false);
                _set.push_back(new_node);
                return new_node;
            }
            // Match found
            return *it;
        }
    }
}
