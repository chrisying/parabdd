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

static int N;                /* Size of the chess board */
static Bdd **X;              /* BDD variable array */
static Bdd queen;            /* N-queen problem express as a BDD */

static void build(int row, int col);

/* Build the requirements for all other fields than (row, col) assuming
   that (row, col) has a queen */
static void build(int row, int col)
{
    std::cout << "// Entered build(row:" << row << ", col:" << col << ")" << std::endl;

    Bdd a = Bdd::bdd_true;
    for (int x = 0 ; x < N ; x++) {
        if (x != col) {
            a &= (X[row][col] > !X[row][x]);
        }
    }
    a.print("Same row");

    Bdd b = Bdd::bdd_true;
    for (int y = 0; y < N; y++) {
        if (y != col) {
            b &= (X[row][col] > !X[y][col]);
        }
    }
    b.print("Same column");

    Bdd c = Bdd::bdd_true;
    for (int k=0 ; k<N ; k++) {
        int ll = k-row+col;
        if (ll>=0 && ll<N) {
            if (k != row) {
                c &= (X[row][col] > !X[k][ll]);
            }
        }
    }
    c.print("Same up-right diagonal");

    Bdd d = Bdd::bdd_true;
    for (int k=0 ; k<N ; k++) {
        int ll = row+col-k;
        if (ll>=0 && ll<N) {
            if (k != row) {
                d &= (X[row][col] > !X[k][ll]);
            }
        }
    }
    d.print("Same down-right diagonal");

    Bdd e = a & b & c & d;
    e.print("All conditions");

    queen &= e;
    queen.print("Combined Queen");
}


int main(int ac, char **argv)
{
    using namespace std ;

    if (ac != 2) {
        std::cerr << "USAGE: " << argv[0] << " N\n";
        return 1;
    }

    N = atoi(argv[1]);
    if (N <= 0) {
        std::cerr << "USAGE: " << argv[0] << " N\n";
        return 1;
    }
    std::cout << "N-queens for " << N << "\n";

    queen = Bdd::bdd_true;

    /* Build variable array */
    X = new Bdd*[N];
    for (int n=0 ; n<N ; n++)
        X[n] = new Bdd[N];

    for (int row = 0 ; row<N ; row++)
        for (int col=0 ; col<N ; col++)
            X[row][col] = Bdd((Variable)(row * N + col));

    /* Place a queen in each row */
    for (int row=0 ; row<N ; row++)
    {
        Bdd e = Bdd::bdd_false;
        for (int col=0 ; col<N ; col++)
            e |= X[row][col];
        queen &= e;
    }
    queen.print("Queens in each row");

    /* Build requirements for each variable(field) */
    for (int row=0 ; row<N ; row++) {
        for (int col=0 ; col<N ; col++) {
            build(row,col);
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
    for (int row = 0; row < N; row++) {
        for (int col = 0; col < N; col++) {
            int index = row*N + col;
            cout << map[static_cast<unsigned int>(index)] << " ";
        }
        cout << endl;
    }

    return 0;
}
