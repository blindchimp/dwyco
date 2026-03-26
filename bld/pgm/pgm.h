#ifndef PGM_H
#define PGM_H

typedef unsigned char gray;

unsigned char **pgm_allocarray(int cols, int rows);
void pgm_freearray(unsigned char **arr, int rows);
unsigned char **pgm_readpgm(const char *path, int *colsP, int *rowsP);
int pgm_writepgm(const char *path, unsigned char **img, int cols, int rows);

#endif /* PGM_H */
