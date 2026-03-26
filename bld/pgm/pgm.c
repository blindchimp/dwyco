#include <stdio.h>
#include <stdlib.h>
#include "pgm.h"

/* Allocates a 2‑D array of gray values (rows x cols) with contiguous memory. */
gray **pgm_allocarray(int cols, int rows) {
    /* Allocate contiguous block for all gray values */
    gray *block = malloc(rows * cols * sizeof(gray));
    if (!block) return NULL;
    /* Allocate row pointer array */
    gray **rows_ptr = malloc(rows * sizeof(gray *));
    if (!rows_ptr) {
        free(block);
        return NULL;
    }
    /* Set each row pointer to the correct offset in the contiguous block */
    for (int r = 0; r < rows; r++) {
        rows_ptr[r] = block + r * cols;
    }
    return rows_ptr;
}

void pgm_freearray(gray **arr, int rows) {
    if (!arr) return;
    /* First free the contiguous data block (pointed to by arr[0]) */
    free(arr[0]);
    /* Then free the row pointer array */
    free(arr);
}

char*
pm_allocrow( int cols, int size )
    {
    char* itrow;

    itrow = (char*) malloc( cols * size );
    if ( itrow == (char*) 0 )
        return 0;
    return itrow;
    }

void
pm_freerow( char *itrow )
    {
    free( itrow );
    }


gray **pgm_readpgm(FILE *fp, int *colsP, int *rowsP) {
    if (!fp) return NULL;
    char format[3];
    if (fscanf(fp, "%2s", format) != 1) { return NULL; }
    if (format[0] != 'P' || format[1] != '5') { return NULL; }
    int maxval = 0;
    int cols, rows;
    int c;
    while ((c = fgetc(fp)) == '#') {
        while ((c = fgetc(fp)) != '\n' && c != EOF);
    }
    if (c != EOF) ungetc(c, fp);
    if (fscanf(fp, "%d %d %d", &cols, &rows, &maxval) != 3) {  return NULL; }
    if (colsP) *colsP = cols;
    if (rowsP) *rowsP = rows;
    //if (maxvalP) *maxvalP = maxval;
    fgetc(fp); // skip single whitespace
    gray **img = pgm_allocarray(cols, rows);
    if (!img) { return NULL; }
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            int val = fgetc(fp);
            if (val == EOF) { pgm_freearray(img, rows); return NULL; }
            img[r][c] = (gray)val;
        }
    }
    return img;
}

int pgm_writepgm(FILE *fp, gray **img, int cols, int rows) {
    if (!fp) return 0;
    int maxval = 255;
    fprintf(fp, "P5\n%d %d\n%d\n", cols, rows, maxval);
    for (int r = 0; r < rows; r++) {
        if (fwrite(img[r], 1, cols, fp) != (size_t)cols) { return 0; }
    }
    return 1;
}

