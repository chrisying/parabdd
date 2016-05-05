/**************************************************************************
  BDD demonstration of the N-Queen chess problem.
  -----------------------------------------------
  The BDD variables correspond to a NxN chess board like:

  0    N      2N     ..  N*N-N
  1    N+1    2N+1   ..  N*N-N+1
  2    N+2    2N+2   ..  N*N-N+2
  ..   ..     ..     ..  ..
  N-1  2N-1   3N-1   ..  N*N-1

  So for example a 4x4 is:

  0 4  8 12
  1 5  9 13
  2 6 10 14
  3 7 11 15

  One solution is then that 2,4,11,13 should be true, meaning a queen
  should be placed there:

  . X . .
  . . . X
  X . . .
  . . X .

 **************************************************************************/
#include <stdlib.h>
#include <set>
#include <unordered_map>
#include <iostream>

#include "bdd.h"

using namespace bdd;

int N;                /* Size of the chess board */
Bdd **X;              /* BDD variable array */
Bdd queen;            /* N-queen problem express as a BDD */

static void build(int x, int y);

/* Build the requirements for all other fields than (x,y) assuming
   that (x,y) has a queen */
static void build(int x, int y)
{
    Bdd a = Bdd::bdd_true;
    Bdd b = Bdd::bdd_true;
    Bdd c = Bdd::bdd_true;
    Bdd d = Bdd::bdd_true;

    std::cout << "// Entered build(" << x << ", " << y << ")" << std::endl;

    /* No one in the same column */
    for (int row = 0 ; row < N ; row++) {
        if (row != y) {
            a &= (X[x][y] > !X[x][row]);
        }
    }
    a.print("a");

    /* No one in the same row */
    for (int col = 0; col < N; col++) {
        if (col != x) {
            b &= (X[x][y] > !X[col][y]);
        }
    }
    b.print("b");

    /* No one in the same up-right diagonal */
    for (int k = 0; k < N; k++) {
        int row = k - x + y;
        if (row >= 0 && row < N && k != x) {
            c &= (X[x][y] > !X[k][row]);
        }
    }
    c.print("c");

    /* No one in the same down-right diagonal */
    for (int k = 0 ; k < N && k <= x + y ; k++) {
        int row = x + y - k;
        if (row >= 0 && row < N && k != x) {
            d &= (X[x][y] > !X[k][row]);
        }
    }
    d.print("d");

    Bdd e = a & b & c & d;
    e.print("e");

    queen &= e;
    queen.print("queen");
}


int main(int ac, char **av)
{
    using namespace std ;

    if (ac != 2) {
        fprintf(stderr, "USAGE:  queen N\n");
        return 1;
    }

    N = atoi(av[1]);
    if (N <= 0) {
        fprintf(stderr, "USAGE:  queen N\n");
        return 1;
    }

    /* Initialize with 100000 nodes, 10000 cache entries and NxN variables */
    queen = Bdd::bdd_true;

    /* Build variable array */
    X = new Bdd*[N];
    for (int n=0 ; n<N ; n++)
        X[n] = new Bdd[N];

    for (int i=0 ; i<N ; i++)
        for (int j=0 ; j<N ; j++)
            X[i][j] = Bdd((Variable)(i*N+j));

    /* Place a queen in each row */
    for (int i=0 ; i<N ; i++)
    {
        Bdd e = Bdd::bdd_false;
        for (int j=0 ; j<N ; j++)
            e |= X[i][j];
        queen &= e;
    }
    queen.print("Queens in each row");

    /* Build requirements for each variable(field) */
    for (int i=0 ; i<N ; i++) {
        for (int j=0 ; j<N ; j++) {
            build(i,j);
        }
    }

    /* Print the results */
    queen.print("N-queens");
    set<Variable> vars;
    for (int i = 0; i < N * N; i++)
        vars.insert((unsigned int)i);
    cout << "There are " << queen.count_sat(vars) << " solutions\n";
    cout << "one is:\n";

    unordered_map<Variable, bool> map = queen.one_sat();
    //cout << map << endl;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            int index = i*N + j;
            cout << map[static_cast<unsigned int>(index)] << " ";
        }
        cout << endl;
    }

    return 0;
}
