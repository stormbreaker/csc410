#ifndef BK_CGL_H
#define BK_CGL_H


// m: rows
// n: columns
// j: iterations
// i: number of cells alive
// k: print every kth iteration (skip k-1 iterations)

void generateAlive(int* alive, int numberAlive, int rows, int columns);

void fillGrid(int* gridRow, int* alive, int numAlive, int myRank, int rowsPerProcess, int columns);

#endif
