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

    //Bdd c(2);
    //Bdd e(3);
    //Bdd f(4);
    //Bdd g(5);
    //Bdd h(6);
    //Bdd i(7);
    //Bdd j(8);
    //Bdd g1 = ((d | c) & (i & j)) ^ ((g > h) < i) & ((e | f) | (c & f & j & i));
    //g1.print();
    //map = g1.one_sat();
    //assert((map[1] || map[2]) && (map[3] || map[4]));

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

    std::set<Variable> s1({1});
    std::set<Variable> s2({1, 2});
    std::set<Variable> s3({1, 2, 3, 4});

    Bdd t = Bdd::bdd_true;
    assert(t.count_sat(s1) == 2);
    assert(t.count_sat(s2) == 4);
    assert(t.count_sat(s3) == 16);

    Bdd f2 = Bdd::bdd_false;
    assert(f2.count_sat(s1) == 0);
    assert(f2.count_sat(s2) == 0);
    assert(f2.count_sat(s3) == 0);

    Bdd n1(1);
    Bdd n2(2);
    Bdd n3(3);
    Bdd n4(4);
    Bdd n5(5);
    Bdd n6(6);

    Bdd t1 = (n1 | n2) & (n3 | n4);
    Bdd t2 = (n1 & n2) | (n3 & n4);
    assert(t1.count_sat(s3) == 9);
    assert(t2.count_sat(s3) == 7);

    std::set<Variable> s4({1,2,3,4,5,6});
    Bdd t3 = ((n1 ^ n2) | !n3) & (((n3 ^ n4)) & !(n6 | n5));
    //std::cout << "t3.count_sat(s4) = " << t3.count_sat(s4) << std::endl;
    //t3.print("t3");
    assert(t3.count_sat(s4) == 6);

    Bdd t4 = n1 > n2;
    assert(t4.count_sat(s2) == 3);

    Bdd t5 = n1 > (!n2 > (n3 > !n4));
    assert(t5.count_sat(s3) == 15);

    Bdd t6 = (n2 | n1) > (n3 & !n1);
    std::cout << "t6 = " << t6.count_sat(s3) << std::endl;
    t6.print("t6");
    assert(t6.count_sat(s3) == 6);

    std::cout << "All tests passed" << std::endl;

	return 0;
}
