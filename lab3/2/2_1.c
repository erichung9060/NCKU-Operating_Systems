#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>

#define matrix_row_x 1234
#define matrix_col_x 250

#define matrix_row_y 250
#define matrix_col_y 4

FILE *fptr1;
FILE *fptr2;
FILE *fptr3;
int **x;
int **y;


// Put file data intp x array
void data_processing(void){
    int tmp;
    fscanf(fptr1, "%d", &tmp);
    fscanf(fptr1, "%d", &tmp);
    for(int i=0; i<matrix_row_x; i++){
        for(int j=0; j<matrix_col_x; j++){
            if (fscanf(fptr1, "%d", &x[i][j])!=1){
                printf("Error reading from file");
                return;
            }
        }
    }

    fscanf(fptr2, "%d", &tmp);
    fscanf(fptr2, "%d", &tmp);
     for(int i=0; i<matrix_row_y; i++){
        for(int j=0; j<matrix_col_y; j++){
            if (fscanf(fptr2, "%d", &y[i][j])!=1){
                printf("Error reading from file");
                return;
            }
        }
    }   
}

void *thread(void *arg){
    int res;
    for(int i=0; i<matrix_row_x; i++){
        for(int j=0; j<matrix_col_y; j++){
            res = 0;
            for(int k=0; k<matrix_row_y; k++){
                /*YOUR CODE HERE*/

                /****************/
            }
            fprintf(fptr3, "%d ", res);
            if(j==matrix_col_y-1) fprintf(fptr3, "\n");        
        }
    }
    return NULL;
}


int main(){
    x = malloc(sizeof(int*)*matrix_row_x);
    for(int i=0; i<matrix_row_x; i++){
        x[i] = malloc(sizeof(int)*matrix_col_x);
    }
    y = malloc(sizeof(int*)*matrix_row_y);
    for(int i=0; i<matrix_row_y; i++){
        y[i] = malloc(sizeof(int)*matrix_col_y);
    }
    fptr1 = fopen("m1.txt", "r");
    fptr2 = fopen("m2.txt", "r");
    fptr3 = fopen("2.txt", "a");
    pthread_t t1;
    data_processing();
    fprintf(fptr3, "%d %d\n", matrix_row_x, matrix_col_y);

    pthread_create(&t1, NULL, thread, NULL);
    pthread_join(t1, NULL);


    fclose(fptr1);
    fclose(fptr2);
    fclose(fptr3);
}
