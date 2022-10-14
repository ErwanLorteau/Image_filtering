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

FILE *openFile(char *fileName) {
    FILE *ifp = fopen(fileName, "r");
    if (ifp == NULL) {
        printf("Error opening the file\n");
        exit(1);
    } else {
        return ifp;
    }
}


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

int compar (const void * a, const void * b)
{
  return (*(int*)a - *(int*)b);
}

int median(int* filter, int filter_size){
    qsort(filter, filter_size*filter_size, sizeof(int), compar);
    return filter[filter_size/2];
}

void applyOnPixel(gray* old, gray* modified, filter_t filter, int filter_size, int i, int j, int nbCols){
    int result = 0;
    for (int k=0; k<filter_size; k++)
        for (int l=0; l<filter_size; l++){ 
            result += old[(i-(filter_size/2)+k) * nbCols + (j-(filter_size/2)+l)] * filter.filterArray[k * filter_size + l];
        }

    modified[i * nbCols + j] = result/filter.weigth;
}

void applyMedianOnPixel(gray* old, gray* modified, int filter_size, int i, int j, int nbCols){
    int filter[filter_size*filter_size];

    for (int k=0; k<filter_size; k++)
        for (int l=0; l<filter_size; l++){
            filter[k * filter_size + l] = old[(i-(filter_size/2)+k) * nbCols + (j-(filter_size/2)+l)];
        }

    modified[i * nbCols + j] = median(filter, filter_size);
}


/**
 * Basic version for a  3*3 filter, with padding on the edge (copying th value)
 */

void applyFilter(gray* old, gray* modified, int nbRows, int nbCols, char filter_type, int filter_size) {
    /**Make a copy in buffer**/

    /**
     * A B C
     * D E F
     * G H I
     */

    filter_t filter; 

    switch(filter_type){
        case 'B': {
            if (filter_size == 3){
                filter = binomial_filter3x3_t;
            } else if (filter_size == 5){
                filter = binomial_filter5x5_t;
            }
            break;
        }

        case 'M':
            break;
        
        default:
            pm_erreur("Unrecognized filter type");
            exit(-1);
    }
 
    for (int i = 0; i < nbRows ; i++) {
        for (int j = 0; j < nbCols; j++) {
            if (j>=filter_size/2 && i>=filter_size/2 && j<nbCols-filter_size/2 && i<nbRows-filter_size/2) {
                if (filter_type == 'B') applyOnPixel(old, modified, filter, filter_size, i, j, nbCols);
                else if (filter_type =='M') applyMedianOnPixel(old, modified, filter_size, i, j, nbCols);
            } else {
                modified[i * nbCols + j] = old[i * nbCols + j];
            }
        }
    }
}



/***Output functions***/
FILE *createFile(char *name) {
    return fopen(name, "w");
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
 * Verify that there is an image file name passed as parameter
 */
void initGuard(int argc) {
    if (argc != 5) {
        pm_erreur("Usage: ./filter image.pgm");
        exit(1);
    }
}

int* compute_histogram(gray* graymap, int nbCols, int nbRows, int maxval){
    int* histogram = (int*)malloc(sizeof(int)*maxval);
    for(int i=0; i<maxval; i++){
        histogram[i]=0;
    }
    for(int i=0; i<nbRows; i++){
        for(int j=0; j<nbCols; j++){
            histogram[graymap[i * nbCols + j]-1]++;
        }
    }
    return histogram;
}

int* compute_cumulative_histogram(int* histogram, int maxval){
    int* cumulative_histogram = (int*)malloc(sizeof(int)*maxval);
    int sum;
    for(int i=0; i<maxval; i++){
        sum = 0;
        for(int j=0; j<=i; j++){
            sum += histogram[j]; 
        }
        cumulative_histogram[i]=sum;
    }
    return cumulative_histogram;
}


int main(int argc, char *argv[]) {
    initGuard(argc) ;


    FILE *file = openFile(argv[1]);
    char filter_type = argv[2][0];
    int filter_size = atoi(argv[3]);
    int nb_smooth = atoi(argv[4]);

    int i, pgmraw, nbRows, nbCols, maxval;
    gray *graymap ;
    int* histogram, *cumulative_histogram;

     /**Read**/
    pgmraw = readPGM(file, &graymap, &nbCols, &nbRows, &maxval);
    
    histogram = compute_histogram(graymap, nbCols, nbRows, maxval);
    cumulative_histogram = compute_cumulative_histogram(histogram, maxval);

    for(int p=0; p<maxval; p++){
        printf("%d ", histogram[p]);
    }

    for(int p=0; p<maxval; p++){
        printf("%d ", cumulative_histogram[p]);
    }



    /**Copy**/
    gray* result = malloc(sizeof(gray)*nbCols*nbRows);

    /**Applying the filter**/
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

    return 0;
}
