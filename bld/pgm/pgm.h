#ifndef PGM_H
#define PGM_H
#include <stdio.h>
typedef unsigned char gray;

unsigned char **pgm_allocarray(int cols, int rows);
void pgm_freearray(unsigned char **arr, int rows);
unsigned char **pgm_readpgm(FILE *, int *colsP, int *rowsP);
int pgm_writepgm(FILE *, unsigned char **img, int cols, int rows);

char* pm_allocrow( int cols, int size );
void pm_freerow( char *itrow );

#endif /* PGM_H */
