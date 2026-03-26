#ifndef PGM_H
#define PGM_H

#include <stdio.h>
typedef unsigned char gray;

gray **pgm_allocarray(int cols, int rows);
void pgm_freearray(gray **arr, int rows);
gray **pgm_readpgm(FILE *, int *colsP, int *rowsP);
int pgm_writepgm(FILE *, gray **img, int cols, int rows);

char* pm_allocrow( int cols, int size );
void pm_freerow( char *itrow );

#endif /* PGM_H */
