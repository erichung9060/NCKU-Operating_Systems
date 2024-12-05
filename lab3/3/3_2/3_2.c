#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>
#include "3_2_Config.h"

#define matrix_row_x 1234
#define matrix_col_x 250

#define matrix_row_y 250
#define matrix_col_y 1234

FILE *fptr1;
FILE *fptr2;
FILE *fptr3;
FILE *fptr4;
FILE *fptr5;
int **x;
int **y;
int **z;
pid_t tid1, tid2;

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

void *thread1(void *arg){
    char data[30];
    sprintf(data, "%s", "Thread 1 says hello!");

#if (THREAD_NUMBER == 1)
    for(int i=0; i<matrix_row_x; i++){
        for(int j=0; j<matrix_col_y; j++){
            for(int k=0; k<matrix_row_y; k++){
                z[i][j] += x[i][k] * y[k][j];
            }      
        }
    }
#elif (THREAD_NUMBER == 2)
    for(int i=0; i<matrix_row_x/2; i++){
        for(int j=0; j<matrix_col_y; j++){
            for(int k=0; k<matrix_row_y; k++){
                z[i][j] += x[i][k] * y[k][j];
            }      
        }
    }
#endif

    /*YOUR CODE HERE*/
    /* Hint: Write data into proc file.*/

    /****************/ 

    char buffer[50]; 
    while (fgets(buffer, sizeof(buffer), fptr4) != NULL){
        printf("%s", buffer);
    }
}


#if (THREAD_NUMBER == 2)
void *thread2(void *arg){
    char data[30];
    sprintf(data, "%s", "Thread 2 says hello!");
    for(int i=matrix_row_x/2; i<matrix_row_x; i++){
        for(int j=0; j<matrix_col_y; j++){
            for(int k=0; k<matrix_row_y; k++){
                z[i][j] += x[i][k] * y[k][j];
            }     
        }
    }
    
    /*YOUR CODE HERE*/
    /* Hint: Write data into proc file.*/

    /****************/   

    char buffer[50]; 
    while (fgets(buffer, sizeof(buffer), fptr5) != NULL){
        printf("%s", buffer);
    } 
}
#endif

int main(){
    char buffer[50];
    x = malloc(sizeof(int*)*matrix_row_x);
    for(int i=0; i<matrix_row_x; i++){
        x[i] = malloc(sizeof(int)*matrix_col_x);
    }
    y = malloc(sizeof(int*)*matrix_row_y);
    for(int i=0; i<matrix_row_y; i++){
        y[i] = malloc(sizeof(int)*matrix_col_y);
    }
    z = malloc(sizeof(int*)*matrix_row_x);
    for(int i=0; i<matrix_row_x; i++){
        z[i] = malloc(sizeof(int)*matrix_col_y);
    }
    fptr1 = fopen("m1.txt", "r");
    fptr2 = fopen("m2.txt", "r");
    fptr3 = fopen("3_2.txt", "a");
    fptr4 = fopen("/proc/Mythread_info", "r");
    fptr5 = fopen("/proc/Mythread_info", "r");

    pthread_t t1, t2;
    data_processing();
    fprintf(fptr3, "%d %d\n", matrix_row_x, matrix_col_y);

    pthread_create(&t1, NULL, thread1, NULL);
#if (THREAD_NUMBER==2)
    pthread_create(&t2, NULL, thread2, NULL);
#endif
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    for(int i=0; i<matrix_row_x; i++){
        for(int j=0; j<matrix_col_y; j++){
            fprintf(fptr3, "%d ", z[i][j]);
            if(j==matrix_col_y-1) fprintf(fptr3, "\n");   
        }
    }
    fclose(fptr1);
    fclose(fptr2);
    fclose(fptr3);
    fclose(fptr4);
    fclose(fptr5);
}
