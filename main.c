#include "Util.h"
#include "filter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Filter initializeFilter() {
    struct Filter filter;
    filter.size_m = 3;
    filter.size_n = 3;
    filter.weigth = 9;
    int filterArray[9] = {1, 2, 1, 2, 4, 2, 1, 2, 1};
    filter.filterArray = filterArray;
    return filter;
}

FILE *openFile(char *fileName) {
    FILE *ifp = fopen(fileName, "r");
    if (ifp == NULL) {
        printf("Error opening the file");
        exit(1);
    } else {
        return ifp;
    }
}


int readPPM(FILE *ifp, gray **graymap, int *n, int *m) {
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
    printf("%d %d \n" , cols, rows) ;
    maxval = pm_getint(ifp);

    /* Memory allocation  */
    *graymap = (gray *)malloc(cols * rows * sizeof(gray));


    /* Reading */
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            if (pgmraw) {
                (*graymap)[i * cols + j] = pm_getrawbyte(ifp); //!
            } else {
                (*graymap)[i * cols + j] = pm_getint(ifp);
            }
        }
    }

    *n = cols;
    *m = rows;
    return pgmraw;
}




/**
 * Basic version for a  3*3 filter, with padding on the edge (copying th value)
 */

gray* deepCopyImage(gray* graymap, int nbRows, int nbCols) {
    gray* deepCopy = malloc (sizeof (gray) * nbCols * nbRows ) ;
    for (int i = 0; i < nbRows; i++) {
        for (int j = 0; j < nbCols; j++) {
            deepCopy[ (nbRows * i) + nbCols ] =  graymap[ (nbRows * i) + nbCols] ;
        }
    }
    return deepCopy ;
}

/**
gray* deepCopyImageWithPadding(gray* graymap, int nbRows, int nbCols, nbRowsPadding, int nbColsPadding) {


    gray* deepCopy = malloc (sizeof (gray) * (nbCols + nbColsPadding) * (nbRows + nbColsPadding) ;
    for (int i = 0 + nbRowsPadding -1 ; i < nbRows; i++) {
        for (int j = 0 + nbColsPadding -1 ; j < nbCols; j++) {
            if (i < nbColsPadding || j/2 < nbRowsPadding) {
                deepCopy[(nbRows * i) + nbCols] = 0 //Here should copy the neeigbourh
            } else {
                deepCopy[(nbRows * i) + nbCols] = graymap[(nbRows * i) + nbCols];
            }
        }
    }
    return deepCopy ;
}
 **/

/**
filterWeigth(struct Filter filter) {
    int sum = 0;
    for (int i = 0 ; i < filter.size_n, i++ ) {
        for (int j = 0 ; i < filter.size_m, j++ ) {
            sum =+ filter.filterArray[(i*filter.size_n) + (j*filter.size_m)]
        }
    }
    return sum ;
}
 **/

gray* applyFilter(gray* old, gray* modified, int nbRows, int nbCols, struct Filter filter) {
    /**Make a copy in buffer**/

    gray* filteredImage = malloc(nbRows * nbCols) ;

    /**
     * A B C
     * D E F
     * G H I
     */

    for (int i = 0; i < nbRows ; i++) {
        for (int j = 0; j < nbCols; j++) {

            if (j>=1 && i>=1 ) {
                modified[(i * nbRows) + j] = (
                                                     (filter.filterArray[0] * old[(i * nbRows - 1) + j - 1]) + //A
                                                     (filter.filterArray[1] * old[(i * nbRows - 1) + j]) + //B
                                                     (filter.filterArray[2] * old[(i * nbRows - 1) + j + 1]) + //C

                                                     (filter.filterArray[3] * old[(i * nbRows) + j - 1]) + //D
                                                     (filter.filterArray[4] * old[(i * nbRows) + j]) +  //E
                                                     (filter.filterArray[5] * old[(i * nbRows) + j + 1]) + //F

                                                     (filter.filterArray[6] * old[(i * nbRows + 1) + j - 1]) + //G
                                                     (filter.filterArray[7] * old[(i * nbRows + 1) + j]) + //H
                                                     (filter.filterArray[8] * old[(i * nbRows + 1) + j + 1])  //I
                                             ) / filter.weigth;
            }
        }
    }
    return filteredImage ;
}



/***Output functions***/
FILE *createFile(char *name) {
    return fopen(name, 'w');
}

void writeInFile(FILE *fp, char *str) {
    fputs(str, fp);
}


/**
 * Print a two dimensionnal array
 */
void printImage(gray* graymap, int nbRows, int nbCols) {
    for (int i = 0; i < nbRows; i++) {
        for (int j = 0; j < nbCols; j++) {
            printf("%c", graymap[i * nbCols + j]);
        }
    }
}

/**
 * Verify that there is an image file name passed as parameter
 */
void initGuard(int argc) {
    if (argc != 2) {
        pm_erreur("Cannot find image");
        exit(1);
    }
}



int main(int argc, char *argv[]) {
    initGuard(argc) ;

    FILE *file = openFile(argv[1]);
    struct Filter filter = initializeFilter() ;

    int pgmraw, nbRows, nbCols;
    gray *graymap ;
     /**Read**/
    pgmraw = readPPM(file, &graymap, &nbRows, &nbRows);
    printf("%d %d", nbRows, nbCols) ;
    /**Copy**/
    gray* copy = deepCopyImage(pgmraw, nbRows, nbRows) ;

    /**Applying the filter**/
    applyFilter(pgmraw, copy, nbRows, nbCols, filter) ;

    /**Creating an output file**/
    FILE* newImage = createFile("newImage.ppm") ;
    writeInFile(newImage, copy) ;
    return 0;
}
