#include <algorithm>

#include "nodeset.h"
#include "node.h"

bdd::internal::Node* NodeSet::lookupOrCreate(const bdd::internal::Node& node) {
    std::list<bdd::internal::Node*>::iterator it = std::find_if(_set.begin(), _set.end(),
            [&](bdd::internal::Node* elem) {
                return (node.root == elem->root && node.branch_true == elem->branch_true && node.branch_false == elem->branch_false);
            });

    if (it == _set.end()) { // No match found, allocate and insert
        bdd::internal::Node* new_node = new bdd::internal::Node(node.root, node.branch_true, node.branch_false);
        _set.push_back(new_node);
        return new_node;
    }
    // Match found
    return *it;
}
