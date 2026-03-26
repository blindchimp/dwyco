
#include <stdio.h>
#include <stdlib.h>

/* Simple PPM/PGM image library – modern, minimal, C99 */

/* Pixel type for PPM */
typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} pixel;

typedef unsigned char pixval;

/* Allocates a 2‑D array of pixels (rows x cols) with contiguous memory. */
pixel **ppm_allocarray(int cols, int rows) {
    /* Allocate contiguous block for all pixels */
    pixel *block = malloc(rows * cols * sizeof(pixel));
    if (!block) return NULL;
    /* Allocate row pointer array */
    pixel **rows_ptr = malloc(rows * sizeof(pixel *));
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

void ppm_freearray(pixel **arr, int rows) {
    if (!arr) return;
    /* First free the contiguous data block (pointed to by arr[0]) */
    free(arr[0]);
    /* Then free the row pointer array */
    free(arr);
}

/* Reads a binary P6 PPM file. Returns pointer to pixel array or NULL on error. */
pixel **ppm_readppm(FILE *fp, int *colsP, int *rowsP) {
    if (!fp) return NULL;
    char format[3];
    if (fscanf(fp, "%2s", format) != 1) { return NULL; }
    if (format[0] != 'P' || format[1] != '6') {
        fclose(fp); return NULL;
    }
    int maxval = 0;
    int cols, rows;
    /* Skip comments */
    int c;
    while ((c = fgetc(fp)) == '#') {
        while ((c = fgetc(fp)) != '\n' && c != EOF);
    }
    if (c != EOF) ungetc(c, fp);
    if (fscanf(fp, "%d %d %d", &cols, &rows, &maxval) != 3) { return NULL; }
    if (colsP) *colsP = cols;
    if (rowsP) *rowsP = rows;
    //if (maxvalP) *maxvalP = maxval;
    /* Skip single whitespace before pixel data */
    fgetc(fp);
    pixel **img = ppm_allocarray(cols, rows);
    if (!img) { return NULL; }
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            unsigned char rgb[3];
            if (fread(rgb, 1, 3, fp) != 3) { ppm_freearray(img, rows); return NULL; }
            img[r][c].r = rgb[0];
            img[r][c].g = rgb[1];
            img[r][c].b = rgb[2];
        }
    }
    return img;
}

/* Writes a binary P6 PPM file. Returns 0 on success, -1 on failure. */
int ppm_writeppm(FILE *fp, pixel **img, int cols, int rows) {
    if (!fp) return -1;
    fprintf(fp, "P6\n%d %d\n%d\n", cols, rows, 255);
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            unsigned char rgb[3] = {img[r][c].r, img[r][c].g, img[r][c].b};
            if (fwrite(rgb, 1, 3, fp) != 3) { return -1; }
        }
    }
    return 0;
}
