// Calculate statiscal variance of bytes in a file
// and graph them on an ASCII circle
//
// v1.0 by circulosmeos, 2015-10.
// wp.me/p2FmmK-96
// goo.gl/TNh5dq
//
// Distributed under GPL v3 License.
//

#include "statistics_circle.h"

#include <stdio.h>
#include <math.h>
#include <stdbool.h>


char sigma_char[MAX_SIGMA_CHAR+1] = { 
    '.', ',', '-', '~', '+', '*', 'o', 'O', '#', '@', 
    '=' 
};

bool color_flag = true;

bool numbers_flag = false;

bool two_circles_flag = false;


int main ( int argc, char *argv[] )
{
    long long total_size = 0;
    long long bytes[256];

    char *SIZE_UNITS[6] = { "bytes", "kiB", "MiB", "GiB", "TiB", "PiB"};
    float readable_size=0.0;

    const int buffer_length = BUFFER_LENGTH;
    char buffer[BUFFER_LENGTH];
    //int i;
    size_t bytes_read;
    
    char *szFile;
    FILE *hFile;

    // sigma
    float mean;
    float sigma;
    float deviation;

    int two_circles_value;

    // for circle generation:
    //int MAX_X=50, MAX_Y=16;
    double complex coordinates[MAX_VALUE+1];
    signed int circle[MAX_X][MAX_Y];
    signed int circle2[MAX_X][MAX_Y];
    int i, j, k;


    // .................................................
    // .................................................
    // .................................................


    if (argc < 2) {
        printf ("\n  circle v1.0 (goo.gl/TNh5dq)\n");
        printf ("\n  Show statistics about bytes contained in a file,\n  as a circle graph of deviations from sigma.\n");
        printf ("  Use:\n  $ %s <filename> [0|1=no color,2=numbers,3=uncoloured numbers] [0-255=two circles!]\n\n", 
            PROGRAM_NAME);
        return 1;
    }

    szFile=(char *)argv[1];

    if (argc >= 3) {
        if (strcmp(argv[2],"1")==0)
            color_flag=false;
        if (strcmp(argv[2],"2")==0)
            numbers_flag=true;
        if (strcmp(argv[2],"3")==0) {
            color_flag=false;
            numbers_flag=true;
        }
    }

    if (argc >= 4) {
        two_circles_flag=true;
        if (strcmp(argv[3],"0")==0) {
            two_circles_value=MAX_VALUE/2;
        } else {
            if(!(two_circles_value = atoi(argv[3]))) {
                printf("Error: Incorrect value for second circle's center (0-255).\n");
                return 2;
            }
        }
    }


    hFile = fopen(szFile, "rb");

    if ( hFile == NULL  ) {
        printf ("Could not open file '%s'\n", szFile);
        return 3;
    }

    // fill counter matrix with zeros
    for (i=0; i<256; ++i)
        bytes[i] = 0;

    // actually count different bytes in file
    do {
        bytes_read = fread(buffer, 1, buffer_length, hFile);
        for (i = 0; i < bytes_read; ++i)
            ++bytes[(unsigned char)buffer[i]];
        total_size += bytes_read;
    } while (bytes_read == buffer_length);

    // close file: it is not needed any more
    close (hFile);


    // .................................................
    // .................................................
    // .................................................


    // sigma calculation:
    sigma = 0.0;
    mean  = 0.0;
    // 1. mean value:
    /*
    for (i=0; i<=MAX_VALUE; i++) {
        mean += (float)bytes[i];
    }
    mean /= (MAX_VALUE+1);
    */
    mean = (float)total_size / (float)(MAX_VALUE+1);
    // 2. sigma value:
    for (i=0; i<=MAX_VALUE; i++) {
        sigma += powf( (float)bytes[i] - mean, 2.0);
    }
    sigma = sqrtf( sigma/(float)(MAX_VALUE+1) );


    // .................................................
    // .................................................
    // .................................................


    create_circle(coordinates);

    // fill circle's container matrix with zeros
    for (i=0; i<MAX_X; i++) {
        for (j=0; j<MAX_Y; j++) {
            circle[i][j]=CIRCLE_EMPTY_VALUE;
            if (two_circles_flag)
                circle2[i][j]=CIRCLE_EMPTY_VALUE;
        }
    }

    // finally!
    // fills circle's container matrix with ASCII art chars
    for (i=0; i<=MAX_VALUE; i++) {

        if (bytes[i] != 0) { //value has been seen in file
        
            // assign character size from sigma deviation
            deviation = ((float)(bytes[i]) - mean) / sigma * 4.0;

            if (fabsf(deviation) >= (float)(MAX_SIGMA_CHAR-1)) {
                if (deviation>0.0)
                    deviation=+(float)(MAX_SIGMA_CHAR-1);
                else
                    deviation=-(float)(MAX_SIGMA_CHAR-1);
            }

            circle[(int)creal(coordinates[i])]
                [(int)cimag(coordinates[i])] = (int)deviation;

        } else { //value hasn't been seen in file

            // mark it with reserved MAX_SIGMA_CHAR value, to be recognized and treated later
            circle[(int)creal(coordinates[i])]
                [(int)cimag(coordinates[i])] = MAX_SIGMA_CHAR;
        
        }
    }

    // if second circle is graphed, it will have the zero displaced
    // so human bias is less likely to occur
    if (two_circles_flag) {
        for (i=0; i<=MAX_VALUE; i++) {
            circle2[(int)creal(coordinates[ (i+two_circles_value)%(MAX_VALUE+1) ])]
                [(int)cimag(coordinates[ (i+two_circles_value)%(MAX_VALUE+1) ])] = 
                    circle[(int)creal(coordinates[i])]
                        [(int)cimag(coordinates[i])];
        }
    }

    // print file name, so output from batch processes is useful:
    printf("%s\n", szFile);

    // actually graph circle on screen with ASCII art chars and colours
    for (j=0; j<MAX_Y; j++) {
        
        for (i=0; i<MAX_X; i++) {
            print_circle_value( circle[i][j] );
        }

        if (two_circles_flag) {
            for (i=0; i<MAX_X; i++) {
                print_circle_value( circle2[i][j] );
            }
        }
        
        printf("\n");    
    }

    // reset colour use in terminal screen
    printf("%s%s", RESET, KWHT);

    if (two_circles_flag) {
        printf("\t    0 centered\t\t\t\t%d centered\n\n", two_circles_value);
    }

    // .................................................
    // .................................................
    // .................................................

    // print various statistics:

    printf("mean =\t%.2f\n", mean);
    printf("sigma= \t%.2f  ", sigma );

    if (mean>0.0) {
        printf("( CV= %.1f%%  )\n", (sigma/mean*100) );
    } else {
        printf("\n");
    }

    readable_size = total_size;
    for (i=0; readable_size>1024; i++)
        readable_size/=1024.0;

    printf("\nsize =\t%.1f %s,  (%lld bytes)\n", readable_size, SIZE_UNITS[i], total_size);

    return 0;

}


void print_circle_value(signed int value) {

    char *color;

    if ( value != CIRCLE_EMPTY_VALUE ) {
        
        if ( value < 0 )
            color=KRED;
        else
            color=KGRN;

        // treat special MAX_SIGMA_CHAR value: it means unseen values
        if (value == MAX_SIGMA_CHAR)
            color=KBLU;

        if (!numbers_flag) {
            if (color_flag)
                printf( "%s%c", color, sigma_char[ abs(value) ] );
            else {
                if (value != MAX_SIGMA_CHAR)
                    printf( "%c", sigma_char[ (value/2) + (int)(MAX_SIGMA_CHAR/2) ] );
                else
                    printf( "%c", sigma_char[ MAX_SIGMA_CHAR ] );
            }
        } else {
            if (color_flag)
                if (value != MAX_SIGMA_CHAR)
                    printf( "%s%c", color, 48+abs(value) );
                else
                    printf( "%s%c", color, sigma_char[ MAX_SIGMA_CHAR ] );
            else {
                if (value != MAX_SIGMA_CHAR)
                    printf( "%c", ( value>=0 ? (48+abs(value)) : (96+abs(value)) ) );
                else
                    printf( "%c", sigma_char[ MAX_SIGMA_CHAR ] );
            }
        }
    } else {
        printf( " " );
    }

}