#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>

#define handle_error_en(en, msg) \
       do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

#define handle_error(msg) \
       do { perror(msg); exit(EXIT_FAILURE); } while (0)

int** mat;
int K, X, N;
typedef struct thread_data{
	int index;    
	int start, end;
} thread_data;
int num_rows_shifted = 0, num_columns_shifted = 0; 
pthread_mutex_t mutex1, mutex2;
pthread_cond_t cond1, cond2;


void *start_routine(void *param)
{
	// Take out the parameters
	struct thread_data *t_param = (struct thread_data *) param;
	int tidx = (*t_param).index;
	int start = (*t_param).start;
	int end = (*t_param).end;

	int itr, itr2;
	for(itr = 0; itr < K; itr++) 
	{
		/* Shift the rows first */
		for(itr2 = start; itr2 <= end; itr2++) 
		{
			int temp = mat[itr2][0], itr3;
			for(itr3 = 0; itr3 < N-1; itr3++)
				mat[itr2][itr3] = mat[itr2][itr3+1];
			mat[itr2][itr3] = temp;
		}
		// printf("HELLO, index %d, K: %d \n", tidx, K);

		pthread_mutex_lock(&mutex1);
		/**************************************MUTEX LOCKED*********************************/
		num_rows_shifted ++;
		/* Signal cond1 to go ahead */
		if(num_rows_shifted % X == 0)  
			pthread_cond_broadcast(&cond1);                        
		pthread_mutex_unlock(&mutex1);
		/*************************************MUTEX UNLOCKED********************************/

		pthread_mutex_lock(&mutex1);
		// we are going to put conditional wait for all rows in a itereation to get shifted
		/**************************************MUTEX LOCKED*********************************/
		while(num_rows_shifted % X != 0)
			pthread_cond_wait(&cond1, &mutex1);
		pthread_mutex_unlock(&mutex1);
		/*************************************MUTEX UNLOCKED********************************/

		for(itr2 = start; itr2 <= end; itr2++)         
		{
			int temp = mat[N-1][itr2];
			int itr3;
			for(itr3 = N-1; itr3 > 0; itr3--)
				mat[itr3][itr2] = mat[itr3-1][itr2];
			mat[itr3][itr2] = temp;
		}

		pthread_mutex_lock(&mutex2);
		/**************************************MUTEX LOCKED*********************************/
		num_columns_shifted++;
		if(num_columns_shifted % X == 0)
			pthread_cond_broadcast(&cond2);
		pthread_mutex_unlock(&mutex2);
		/*************************************MUTEX UNLOCKED********************************/

		pthread_mutex_lock(&mutex2);
		while(num_columns_shifted % X != 0)
			pthread_cond_wait(&cond2, &mutex2);
		pthread_mutex_unlock(&mutex2);
	}
	pthread_exit(NULL);
}


int main(){
	int itr, itr2;
	// printf("Enter N: \n");
	scanf("%d", &N);

	/* Allocating space to int** mat */
	mat = (int **)malloc(N*sizeof(int *));
	for(itr = 0; itr < N; itr++){
		mat[itr] = (int *)malloc(N*sizeof(int));
	}


	/* Reading matrix */
	for(itr = 0; itr < N; itr ++){
		for(itr2 = 0; itr2 < N; itr2++){
			scanf("%d", &mat[itr][itr2]);
		}
	}

	/* Printing matrix */
	printf("\nGiven matrix of %d x %d dimension: \n", N, N);
	for(itr = 0; itr < N; itr ++){
		printf("\n");
		for(itr2 = 0; itr2 < N; itr2++){
			printf("%d ", mat[itr][itr2]);
		}
	}
	printf("\n\n");

	/* Take user input for K and X */
	scanf("%d %d",&K, &X);

	/* Initializing mutex locks */
	pthread_mutex_init(&mutex1, NULL);
	pthread_mutex_init(&mutex2, NULL); 
	pthread_cond_init(&cond1,NULL); 
	pthread_cond_init(&cond2,NULL);
	/* done */

	/* We need to use exactly X threads for the task */
	pthread_t thread_id[X];  
	/* Structure for storing thread information / parameters */
	struct thread_data param[X];

	int d = N/X;
	for(itr = 0; itr < X; itr ++){
		param[itr].index = itr;
		param[itr].start = d * itr;
		if (itr == X-1)
			param[itr].end = N-1;
		else
			param[itr].end = d*itr + d - 1;
	}

	for(itr = 0; itr < X; itr ++){
		pthread_create(&thread_id[itr], NULL, start_routine, (void *) &param[itr]);
	}

	/* Wait for all threads to terminate */
	for(itr = 0; itr < X; itr++)
		pthread_join(thread_id[itr], NULL); 


	// The lock is actually unnecessary here as all other threads
	// must have exited. But just good practice; in a general setting
	// there can be other threads accessing it other than the ones
	// this process waits for
	/****************************************************MUTEX LOCKED********************************************/
	pthread_mutex_lock(&mutex1);
	
	/* Printing the matrix */
	printf("\n\n%d-shuffle of given matrix of %d x %d dimension: \n",K, N, N);

	for(itr = 0; itr < N; itr ++){
		printf("\n");
		for(itr2 = 0; itr2 < N; itr2++){
			printf("%d ", mat[itr][itr2]);
		}
	}
	printf("\n\n");

	/* De-allocating the space for the matrix */
	for(itr = 0; itr < N; itr ++){
		free(mat[itr]); 
	}
	free(mat);

	pthread_mutex_unlock(&mutex1);
	/****************************************************MUTEX UNLOCKED********************************************/


	/* Destroy/ clean up all thread semaphores */
	pthread_mutex_destroy(&mutex1);
	pthread_mutex_destroy(&mutex2);  
	pthread_cond_destroy(&cond1);
	pthread_cond_destroy(&cond2);

	return 0;
}