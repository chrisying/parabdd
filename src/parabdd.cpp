
#include <unordered_map>
#include <iostream>

#include "bdd.h"

int main() {

    using namespace bdd;
    Bdd a(1);
    Bdd b(2);
    Bdd c = a | b;

    std::unordered_map<Variable, bool> map = c.one_sat();
    std::out << "Var 1: " << map[1] << ", Var 2: " << map[2] << std::endl;

	return 0;
}
