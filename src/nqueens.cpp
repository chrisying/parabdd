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


/* Build the requirements for all other fields than (i,j) assuming
   that (i,j) has a queen */
static void build(int i, int j)
{
    Bdd a(true);
    Bdd b(true);
    Bdd c(true);
    Bdd d(true);
    int k,l;

    /* No one in the same column */
    for (l=0 ; l<N ; l++)
        if (l != j)
            a = a & (X[i][j] > !X[i][l]);

    /* No one in the same row */
    for (k=0 ; k<N ; k++)
        if (k != i)
            b = b & (X[i][j] > !X[k][j]);

    /* No one in the same up-right diagonal */
    for (k=0 ; k<N ; k++)
    {
        int ll = k-i+j;
        if (ll>=0 && ll<N)
            if (k != i)
                c = c & (X[i][j] > !X[k][ll]);
    }

    /* No one in the same down-right diagonal */
    for (k=0 ; k<N ; k++)
    {
        int ll = i+j-k;
        if (ll>=0 && ll<N)
            if (k != i)
                d = d & (X[i][j] > !X[k][ll]);
    }

    queen = queen & (a & b & c & d);
}


int main(int ac, char **av)
{
    using namespace std ;
    int n,i,j;

    if (ac != 2)
    {
        fprintf(stderr, "USAGE:  queen N\n");
        return 1;
    }

    N = atoi(av[1]);
    if (N <= 0)
    {
        fprintf(stderr, "USAGE:  queen N\n");
        return 1;
    }

    /* Initialize with 100000 nodes, 10000 cache entries and NxN variables */
    queen = new Bdd(true);

    /* Build variable array */
    X = new Bdd*[N];
    for (n=0 ; n<N ; n++)
        X[n] = new Bdd[N];

    for (i=0 ; i<N ; i++)
        for (j=0 ; j<N ; j++)
            X[i][j] = Bdd((Variable)(i*N+j));

    /* Place a queen in each row */
    for (i=0 ; i<N ; i++)
    {
        Bdd e(false);
        for (j=0 ; j<N ; j++)
            e = e | X[i][j];
        queen = queen & e;
    }

    /* Build requirements for each variable(field) */
    for (i=0 ; i<N ; i++)
        for (j=0 ; j<N ; j++)
        {
            cout << "Adding position " << i << "," << j << "\n" << flush;
            build(i,j);
        }

    /* Print the results */
    set<Variable> vars;
    for (i = 0; i < N * N; i++)
        vars.insert((unsigned int)i);
    cout << "There are " << queen.count_sat(vars) << " solutions\n";
    cout << "one is:\n";

    unordered_map<Variable, bool> map = queen.one_sat();
    //cout << map << endl;
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            cout << map[i*N + j] << " ";
        }
        cout << endl;
    }

    return 0;
}
