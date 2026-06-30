#ifndef PGM_H
#define PGM_H

typedef unsigned char gray;

gray **pgm_allocarray(int cols, int rows);
void pgm_freearray(gray **arr, int rows);
#if 0
gray **pgm_readpgm(FILE *, int *colsP, int *rowsP);
int pgm_writepgm(FILE *, gray **img, int cols, int rows);
#endif

char* pm_allocrow( int cols, int size );
void pm_freerow( char *itrow );

#endif /* PGM_H */
