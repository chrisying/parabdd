#include <cassert>
#include <unordered_map>
#include <iostream>
#include <set>

#include "bdd.h"

int main() {

    using namespace bdd;
    Bdd a(1);
    Bdd b(2);

    std::unordered_map<Variable, bool> map = a.one_sat();
    assert(map[1]);

    Bdd c1 = a | b;
    map = c1.one_sat();
    assert(map[1] || map[2]);

    Bdd c2 = !a;
    map = c2.one_sat();
    assert(!map[1]);

    Bdd c3 = a & !b;
    map = c3.one_sat();
    assert(map[1] && !map[2]);

    Bdd d(1);
    Bdd c(2);
    Bdd e(3);
    Bdd f(4);
    Bdd g(5);
    Bdd h(6);
    Bdd i(7);
    Bdd j(8);
    Bdd g1 = ((d | c) & (i & j)) ^ ((g > h) < i) & ((e | f) | (c & f & j & i));
    g1.print();
    map = g1.one_sat();
    assert((map[1] || map[2]) && (map[3] || map[4]));

    std::set<Variable> av({1});
    int ca = a.count_sat(av);
    assert(ca == 1);

    std::set<Variable> c1v({1, 2});
    int cc1 = c1.count_sat(c1v);
    assert(cc1 == 3);

    std::set<Variable> c1v2({1, 2, 3});
    int cc12 = c1.count_sat(c1v2);
    assert(cc12 == 6);

    std::set<Variable> c3v({1, 2});
    int cc3 = c3.count_sat(c3v);
    assert(cc3 == 1);

    std::cout << "All tests passed" << std::endl;

	return 0;
}
