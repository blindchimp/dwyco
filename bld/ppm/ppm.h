#ifndef PPM_H
#define PPM_H

typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} pixel;

pixel **ppm_allocarray(int cols, int rows);
void ppm_freearray(pixel **arr, int rows) ;
pixel **ppm_readppm(const char *path, int *colsP, int *rowsP) ;
int ppm_writeppm(const char *path, pixel **img, int cols, int rows) ;

#define PPM_GETR(p) ((p).r)
#define PPM_GETG(p) ((p).g)
#define PPM_GETB(p) ((p).b)
#define PPM_ASSIGN(p,red,grn,blu) do { (p).r = (red); (p).g = (grn); (p).b = (blu); } while ( 0 )

#endif /* PPM_H */
