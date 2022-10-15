#include "Util.h"
#include "filter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int binomial_filter3x3[9] = {1, 2, 1,
                             2, 4, 2,
                             1, 2, 1};

filter_t binomial_filter3x3_t = {
    .filterArray = binomial_filter3x3,
    .size_m = 3,
    .size_n = 3,
    .weigth = 16
};

int binomial_filter5x5[25] = {1, 4, 6, 4, 1,
                              4, 16, 24, 16, 4,
                              6, 24, 36, 24, 6,
                              4, 16, 24, 16, 4,
                              1, 4, 6, 4, 1};

filter_t binomial_filter5x5_t = {
    .filterArray = binomial_filter5x5,
    .size_m = 5, 
    .size_n = 5, 
    .weigth = 256
};

/* Opens a file */
FILE *openFile(char *fileName) {
    FILE *ifp = fopen(fileName, "r");
    if (ifp == NULL) {
        printf("Error opening the file\n");
        exit(1);
    } else {
        return ifp;
    }
}


/* Reads the PGM image into graymap */
int readPGM(FILE *ifp, gray **graymap, int *n, int *m, int *max) {
    /* Magic number reading */
    int ich1, ich2, maxval = 255, cols, rows, pgmraw;
    int i, j;

    ich1 = getc(ifp);
    if (ich1 == EOF)
        pm_erreur("EOF / read error / magic number");
    ich2 = getc(ifp);
    if (ich2 == EOF)
        pm_erreur("EOF /read error / magic number");
    if (ich2 != '2' && ich2 != '5')
        pm_erreur(" wrong file type ");
    else if (ich2 == '2')
        pgmraw = 0;
    else
        pgmraw = 1;

    /* Reading image dimensions */
    cols = pm_getint(ifp);
    rows = pm_getint(ifp);
    maxval = pm_getint(ifp);

    /* Memory allocation  */
    *graymap = (gray *)malloc(cols * rows * sizeof(gray));


    /* Reading */
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            if (pgmraw) {
                (*graymap)[i * cols + j] = pm_getrawbyte(ifp);
            } else {
                (*graymap)[i * cols + j] = pm_getint(ifp);
            }
        }
    }

    *n = cols;
    *m = rows;
    *max = maxval;
    return pgmraw;
}

/* Comparison function by qsort */
int compar (const void * a, const void * b)
{
  return (*(int*)a - *(int*)b);
}

/* Finds the median of an array (filter) 
* The function uses quick sort to sort the array and returns the value in the middle */
int median(int* filter, int filter_size){
    qsort(filter, filter_size*filter_size, sizeof(int), compar);
    return filter[filter_size/2];
}

/* Applies a filter of size filter_size (3 or 5) on a pixel */
void applyOnPixel(gray* old, gray* modified, filter_t filter, int filter_size, int i, int j, int nbCols){
    int result = 0;
    for (int k=0; k<filter_size; k++)
        for (int l=0; l<filter_size; l++){ 
            result += old[(i-(filter_size/2)+k) * nbCols + (j-(filter_size/2)+l)] * filter.filterArray[k * filter_size + l];
        }

    modified[i * nbCols + j] = result/filter.weigth;
}

/* Finds the median out of the pixel's neighbours and modifies its value */
void applyMedianOnPixel(gray* old, gray* modified, int filter_size, int i, int j, int nbCols){
    int filter[filter_size*filter_size];

    for (int k=0; k<filter_size; k++)
        for (int l=0; l<filter_size; l++){
            filter[k * filter_size + l] = old[(i-(filter_size/2)+k) * nbCols + (j-(filter_size/2)+l)];
        }

    modified[i * nbCols + j] = median(filter, filter_size);
}


/* Applies a filter on an image (old) in the (modified) image */
void applyFilter(gray* old, gray* modified, int nbRows, int nbCols, char filter_type, int filter_size) {
    /**Make a copy in buffer**/

    /**
     * A B C
     * D E F
     * G H I
     */

    filter_t filter; 

    /* Checks the filter type entered in the program arguments */
    switch(filter_type){
        case 'B':
            if (filter_size == 3){
                filter = binomial_filter3x3_t;
            } else if (filter_size == 5){
                filter = binomial_filter5x5_t;
            }
            break;

        case 'M':
            break;
        
        default:
            pm_erreur("Unrecognized filter type");
            exit(-1);
    }

    /* Applies the filter on every pixel in the image */
    for (int i = 0; i < nbRows ; i++) {
        for (int j = 0; j < nbCols; j++) {
            /* Checks if it is not an edge */
            if (j>=filter_size/2 && i>=filter_size/2 && j<nbCols-filter_size/2 && i<nbRows-filter_size/2) {
                if (filter_type == 'B') applyOnPixel(old, modified, filter, filter_size, i, j, nbCols);
                else if (filter_type =='M') applyMedianOnPixel(old, modified, filter_size, i, j, nbCols);
            } else {
            /* If it is an edge, the values are not changed */
                modified[i * nbCols + j] = old[i * nbCols + j];
            }
        }
    }
}

/***Output functions***/
FILE *createFile(char *name) {
    FILE* ifp = fopen(name, "w");
    if (ifp == NULL){
        printf("Error opening the file\n");
        exit(1);
    }
    return ifp;
}

void writeInFile(FILE *fp, gray* graymap, int pgmraw, int nbCols, int nbRows, int maxval) {

    if(pgmraw)
       fprintf(fp, "P2\n");
    else
        fprintf(fp, "P5\n");

    fprintf(fp, "%d %d \n", nbCols, nbRows);
    fprintf(fp, "%d\n",maxval);

    for(int i=0; i < nbRows; i++)
      for(int j=0; j < nbCols ; j++){
        if(pgmraw)
           fprintf(fp,"%d ", graymap[i * nbCols + j]);
        else
            fprintf(fp,"%c",graymap[i * nbCols + j]);
      }

    /* Closing */
    fclose(fp);
}


/**
 * Print a two dimensionnal array
 */
void printImage(gray* graymap, int nbRows, int nbCols) {
    for (int i = 0; i < nbRows; i++) {
        for (int j = 0; j < nbCols; j++) {
            printf("%d", graymap[i * nbCols + j]);
        }
    }
}

/**
 * Verify arguments passed to program
 */
void initGuard(int argc) {
    if (argc != 7) {
        pm_erreur("Usage: ./filter image.pgm FILTER FILTER_SIZE NB_SMOOTH HISTO_STRETCH HISTO_EQUALIZATION");
        exit(1);
    }
}

/* Computes the histogram of grayamp */
int* compute_histogram(gray* graymap, int nbCols, int nbRows, int maxval){
    int* histogram = (int*)malloc(sizeof(int)*maxval+1);
    for(int i=0; i<=maxval; i++){
        histogram[i]=0;
    }
    for(int i=0; i<nbRows; i++){
        for(int j=0; j<nbCols; j++){
            histogram[graymap[i * nbCols + j]]++;
        }
    }
    return histogram;
}

/* Computes the cumulative histogram of an intensity histogram */
int* compute_cumulative_histogram(int* histogram, int maxval){
    int* cumulative_histogram = (int*)malloc(sizeof(int)*maxval+1);
    int sum;
    for(int i=0; i<=maxval; i++){
        sum = 0;
        for(int j=0; j<=i; j++){
            sum += histogram[j]; 
        }
        cumulative_histogram[i]=sum;
    }
    return cumulative_histogram;
}


/* Finds the image intensity range and stores the min value in i1 and the max value in i2 */
void image_dynamic(int* histogram, int maxval, int* i1, int* i2){
    int i=0, j=maxval;

    *i1 = 0;
    *i2 = 0;

    while (i<=maxval && *i1 == 0){
        if (histogram[i] > 0){
            *i1 = i;
        }
        i++;
    }
    while (j>=0 && *i2 == 0){
        if (histogram[j] > 0){
            *i2 = j;
        }
        j--;
    }
} 

/* Applies an linear transform on an intensity value */
int linear_transform(int intensity, int min_intensity, int max_intensity, int maxval){
    return maxval*(intensity - min_intensity)/(max_intensity - min_intensity);
}

/* Stretches the histogram by applying the linear transform for each pixel */
void histogram_stretch(gray* graymap, int maxval, int nbRows, int nbCols, int min_intensity, int max_intensity){
    for(int i=0; i<nbRows; i++){
        for(int j=0; j<nbCols; j++){
            graymap[i * nbCols + j] = linear_transform(graymap[i * nbCols + j],min_intensity, max_intensity, maxval);
        }
    }
}

/* Applies a non linear transform on a given intensity value to achieve the optimal cumulative distribution */
int non_linear_transform(int* cumulative_histogram, int intensity, int maxval){
    return maxval*cumulative_histogram[intensity]/cumulative_histogram[maxval];
}

/* Performs a histogram equalization by applying the non-linear transform on each pixel */
void histogram_equalization(gray* graymap, int maxval, int nbRows, int nbCols, int* cumulative_histogram){
    for(int i=0; i<nbRows; i++){
        for(int j=0; j<nbCols; j++){
            graymap[i * nbCols + j] = non_linear_transform(cumulative_histogram, graymap[i * nbCols + j], maxval);
        }
    }
}

int main(int argc, char *argv[]) {
    initGuard(argc) ;

    FILE *file = openFile(argv[1]);
    char filter_type = argv[2][0];
    int filter_size = atoi(argv[3]);
    int nb_smooth = atoi(argv[4]);
    int stretch = atoi(argv[5]);
    int equalization = atoi(argv[6]);

    int i, pgmraw, nbRows, nbCols, maxval, min_intensity, max_intensity;
    gray *graymap ;
    int* histogram, *cumulative_histogram;

    /**Read**/
    pgmraw = readPGM(file, &graymap, &nbCols, &nbRows, &maxval);

    /* Compute intensity histogram and cumulative histogram*/
    histogram = compute_histogram(graymap, nbCols, nbRows, maxval);
    cumulative_histogram = compute_cumulative_histogram(histogram, maxval);

    /* Compute image intensity range and store in min_intensity and max_intensity */
    image_dynamic(histogram, maxval, &min_intensity, &max_intensity);

    if (stretch != 0){
        histogram_stretch(graymap, maxval, nbRows, nbCols, min_intensity, max_intensity);
        histogram = compute_histogram(graymap, nbCols, nbRows, maxval);
        cumulative_histogram = compute_cumulative_histogram(histogram, maxval);
    }

    if (equalization != 0){
        histogram_equalization(graymap, maxval, nbRows, nbCols, cumulative_histogram);
        histogram = compute_histogram(graymap, nbCols, nbRows, maxval);
        cumulative_histogram = compute_cumulative_histogram(histogram, maxval);
    }

    /**Copy**/
    gray* result = malloc(sizeof(gray)*nbCols*nbRows);

    /* Applying the filter nb_smooth times
    The filtered image is stored in: result if nb_smooth is odd, 
    otherwise it is sotred in: graymap
    */
    for (i=0; i<nb_smooth; i++){
        if (i%2==0){
            applyFilter(graymap, result, nbRows, nbCols, filter_type, filter_size) ;
        } else {
            applyFilter(result, graymap, nbRows, nbCols, filter_type, filter_size) ;
        }       
    }

    /**Creating an output file**/
    FILE* newImage = createFile("result.pgm") ;

    if (i%2==1){
        writeInFile(newImage, result, pgmraw, nbCols, nbRows, maxval) ;
    } else {
        writeInFile(newImage, graymap, pgmraw, nbCols, nbRows, maxval) ;
    }

    /* Free dynamically allocated blocks */
    free(result);
    free(graymap);
    free(histogram);
    free(cumulative_histogram);

    /* Close open files */
    fclose(file);

    return 0;
}
