#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#include "project.h"

#define SEGMENTSIZE sizeof(sem_t)
#define SEGMENTPERM 0666

extern int errno;

int main(int argc, char **argv)
{

	int retval;
	int id, err;
	int lb, ub;
	int saladmaker_number;
	struct shared_memory *s;
	FILE *fp;
	FILE *fp_saladmaker1;
	FILE *fp_saladmaker2;
	FILE *fp_saladmaker3;
	struct timeval now;
	struct tm *local;

	// το χρονικό διάστημα που χρειάζεται ο saladmaker
	// για να κάνει τη σαλάτα
	lb = atoi(argv[2]);
	ub = atoi(argv[4]);

	// το shmid
	id = atoi(argv[6]);

	// σε ποιόν από τους 3 saladmakers βρισκόμαστε
	saladmaker_number = atoi(argv[7]);
	

	/* Attach the segment. */
	s = (struct shared_memory *)shmat(id, (void *)0, 0);
	if (s == (void *)-1)
	{
		perror("Attachment.");
		exit(2);
	}

	// άνοιγμα  ενιαίου αρχείου
	fp = fopen("single.txt", "a");
	// έλεγχος
    if(fp == NULL){

        printf("Error: fopen() failed\n");
        exit(EXIT_FAILURE);
    }

	// άνοιγμα  αρχείου saladmaker1
	fp_saladmaker1 = fopen("saladmaker1.txt", "a");
	// έλεγχος
    if(fp_saladmaker1 == NULL){

        printf("Error: fopen() failed\n");
        exit(EXIT_FAILURE);
    }

	// άνοιγμα αρχείου saladmaker2
	fp_saladmaker2 = fopen("saladmaker2.txt", "a");
	// έλεγχος
    if(fp_saladmaker2 == NULL){

        printf("Error: fopen() failed\n");
        exit(EXIT_FAILURE);
    }

	// άνοιγμα αρχείου saladmaker2
	fp_saladmaker3 = fopen("saladmaker3.txt", "a");
	// έλεγχος
    if(fp_saladmaker3 == NULL){

        printf("Error: fopen() failed\n");
        exit(EXIT_FAILURE);
    }


	// αν η παραγγελία δόθηκε για τον saladmaker1
	if (saladmaker_number == 1)
	{

		// ενημέρωση του pid
		s->saladmaker1_pid = getpid();

		while (1)
		{

			///////////////////////////////////////////////////////////////////////////////
			gettimeofday(&now, NULL);
			local = localtime(&now.tv_sec);

			// γράψιμο στο αρχείο αφού έχει εξασφαλιστεί ότι γράφει μόνο
			// αυτός στο αρχείο
			sem_wait(&(s->mutex));
			fprintf(fp, "[%02d:%02d:%02d.%03ld][%d][Saladmaker1][Waiting for ingredients]\n", local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, s->saladmaker1_pid);
			fflush(fp);
			sem_post(&(s->mutex));
			////////////////////////////////////////////////////////////////////////////

			// γράψιμο στο αρχείο saladmaker1.txt
			fprintf(fp_saladmaker1, "[%02d:%02d:%02d.%03ld][%d][Saladmaker1][Waiting for ingredients]\n", local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, s->saladmaker1_pid);
			fflush(fp_saladmaker1);


			// αναμένει να ειδοποιηθεί από το chef
			sem_wait(&(s->saladmaker1));

			// έλεγχος για το αν χρειάζεται να γίνουν και άλλες σαλάτες
			if(s->remaining_salads <= 0){

				break;
			}

			////////////////////////////////////////////////////////////////
			gettimeofday(&now, NULL);
			local = localtime(&now.tv_sec);

			// γράψιμο στο αρχείο αφού έχει εξασφαλιστεί ότι γράφει μόνο
			// αυτός στο αρχείο
			sem_wait(&(s->mutex));
			fprintf(fp, "[%02d:%02d:%02d.%03ld][%d][Saladmaker1][Get ingredients]\n", local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, s->saladmaker1_pid);
			fflush(fp);
			sem_post(&(s->mutex));
			/////////////////////////////////////////////////////////////////////////////

			// γράψιμο στο αρχείο saladmaker1.txt
			fprintf(fp_saladmaker1, "[%02d:%02d:%02d.%03ld][%d][Saladmaker1][Get ingredients]\n", local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, s->saladmaker1_pid);
			fflush(fp_saladmaker1);


			// ειδοποιεί τον chef ότι πήρε τα υλικά
			sem_post(&(s->products_taken));

			////////////////////////////////////////////////////////////////////////////
			gettimeofday(&now, NULL);
			local = localtime(&now.tv_sec);

			// γράψιμο στο αρχείο αφού έχει εξασφαλιστεί ότι γράφει μόνο
			// αυτός στο αρχείο
			sem_wait(&(s->mutex));
			fprintf(fp, "[%02d:%02d:%02d.%03ld][%d][Saladmaker1][Start making salad]\n", local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, s->saladmaker1_pid);
			fflush(fp);
			sem_post(&(s->mutex));
			/////////////////////////////////////////////////////////////////////////////////

			// γράψιμο στο αρχείο saladmaker1.txt
			fprintf(fp_saladmaker1, "[%02d:%02d:%02d.%03ld][%d][Saladmaker1][Start making salad]\n", local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, s->saladmaker1_pid);
			fflush(fp_saladmaker1);

			// παρασκευή της σαλάτας
			sleep(ub-lb);
			

			////////////////////////////////////////////////////////////////////////////
			gettimeofday(&now, NULL);
			local = localtime(&now.tv_sec);

			// γράψιμο στο αρχείο 
			// αφού έχει εξασφαλιστεί ότι γράφει μόνο
			// αυτός στο αρχείο
			sem_wait(&(s->mutex));
			s->remaining_salads--;	// ενημέωρση της μεταβλητής s->remaining_salads
			fprintf(fp, "[%02d:%02d:%02d.%03ld][%d][Saladmaker1][End making salad]\n", local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, s->saladmaker1_pid);
			fflush(fp);
			sem_post(&(s->mutex));
			/////////////////////////////////////////////////////////////////////////////////

			// γράψιμο στο αρχείο saladmaker1.txt
			fprintf(fp_saladmaker1, "[%02d:%02d:%02d.%03ld][%d][Saladmaker1][End making salad]\n", local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, s->saladmaker1_pid);
			fflush(fp_saladmaker1);

			// αύξηση κατά 1 των σαλατών που έχει φτιάξει ο saladmaker1
			s->saladmaker1_made_salads++;

			// έλεγχος για το αν χρειάζεται να γίνουν και άλλες σαλάτες
			if(s->remaining_salads <= 0){

				break;
			}
		}
	}
	// αν η παραγγελία δόθηκε για τον saladmaker2
	else if (saladmaker_number == 2)
	{

		// ενημέρωση του pid
		s->saladmaker2_pid = getpid();

		while (1)
		{

			////////////////////////////////////////////////////////////////////////////////
			gettimeofday(&now, NULL);
			local = localtime(&now.tv_sec);

			// γράψιμο στο αρχείο αφού έχει εξασφαλιστεί ότι γράφει μόνο
			// αυτός στο αρχείο
			sem_wait(&(s->mutex));
			fprintf(fp, "[%02d:%02d:%02d.%03ld][%d][Saladmaker2][Waiting for ingredients]\n", local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, s->saladmaker2_pid);
			fflush(fp);
			sem_post(&(s->mutex));
			//////////////////////////////////////////////////////////////////////////////////

			// γράψιμο στο αρχείο saladmaker2.txt
			fprintf(fp_saladmaker2, "[%02d:%02d:%02d.%03ld][%d][Saladmaker2][Waiting for ingredients]\n", local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, s->saladmaker2_pid);
			fflush(fp_saladmaker2);

			// αναμένει να ειδοποιηθεί από το chef
			sem_wait(&(s->saladmaker2));

			// έλεγχος για το αν χρειάζεται να γίνουν και άλλες σαλάτες
			if(s->remaining_salads <= 0){

				break;
			}

			////////////////////////////////////////////////////////////////
			gettimeofday(&now, NULL);
			local = localtime(&now.tv_sec);

			// γράψιμο στο αρχείο αφού έχει εξασφαλιστεί ότι γράφει μόνο
			// αυτός στο αρχείο
			sem_wait(&(s->mutex));
			fprintf(fp, "[%02d:%02d:%02d.%03ld][%d][Saladmaker2][Get ingredients]\n", local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, s->saladmaker2_pid);
			fflush(fp);
			sem_post(&(s->mutex));
			/////////////////////////////////////////////////////////////////////////////

			// γράψιμο στο αρχείο saladmaker2.txt
			fprintf(fp_saladmaker2, "[%02d:%02d:%02d.%03ld][%d][Saladmaker2][Get ingredients]\n", local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, s->saladmaker2_pid);
			fflush(fp_saladmaker2);

			// ειδοποιεί τον chef ότι πήρε τα υλικά
			sem_post(&(s->products_taken));

			////////////////////////////////////////////////////////////////////////////
			gettimeofday(&now, NULL);
			local = localtime(&now.tv_sec);

			// γράψιμο στο αρχείο αφού έχει εξασφαλιστεί ότι γράφει μόνο
			// αυτός στο αρχείο
			sem_wait(&(s->mutex));
			fprintf(fp, "[%02d:%02d:%02d.%03ld][%d][Saladmaker2][Start making salad]\n", local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, s->saladmaker2_pid);
			fflush(fp);
			sem_post(&(s->mutex));
			/////////////////////////////////////////////////////////////////////////////////

			// γράψιμο στο αρχείο saladmaker2.txt
			fprintf(fp_saladmaker2, "[%02d:%02d:%02d.%03ld][%d][Saladmaker2][Start making salad]\n", local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, s->saladmaker2_pid);
			fflush(fp_saladmaker2);

			// παρασκευή της σαλάτας
			sleep(ub-lb);
			
			////////////////////////////////////////////////////////////////////////////
			gettimeofday(&now, NULL);
			local = localtime(&now.tv_sec);

			// γράψιμο στο αρχείο αφού έχει εξασφαλιστεί ότι γράφει μόνο
			// αυτός στο αρχείο
			sem_wait(&(s->mutex));
			s->remaining_salads--;	// ενημέωρση της μεταβλητής s->remaining_salads
			fprintf(fp, "[%02d:%02d:%02d.%03ld][%d][Saladmaker2][End making salad]\n", local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, s->saladmaker2_pid);
			fflush(fp);
			sem_post(&(s->mutex));
			/////////////////////////////////////////////////////////////////////////////////

			// γράψιμο στο αρχείο saladmaker2.txt
			fprintf(fp_saladmaker2, "[%02d:%02d:%02d.%03ld][%d][Saladmaker2][End making salad]\n", local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, s->saladmaker2_pid);
			fflush(fp_saladmaker2);

			// αύξηση κατά 1 των σαλατών που έχει φτιάξει ο saladmaker2
			s->saladmaker2_made_salads++;

			// έλεγχος για το αν χρειάζεται να γίνουν και άλλες σαλάτες
			if(s->remaining_salads <= 0){

				break;
			}
		}
	}
	// αν η παραγγελία δόθηκε για τον saladmaker3
	else
	{

		// ενημέρωση του pid
		s->saladmaker3_pid = getpid();

		while (1)
		{

			///////////////////////////////////////////////////////
			gettimeofday(&now, NULL);
			local = localtime(&now.tv_sec);

			// γράψιμο στο αρχείο αφού έχει εξασφαλιστεί ότι γράφει μόνο
			// αυτός στο αρχείο
			sem_wait(&(s->mutex));
			fprintf(fp, "[%02d:%02d:%02d.%03ld][%d][Saladmaker3][Waiting for ingredients]\n", local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, s->saladmaker3_pid);
			fflush(fp);
			sem_post(&(s->mutex));
			//////////////////////////////////////////////////////////////////////////

			// γράψιμο στο αρχείο saladmaker3.txt
			fprintf(fp_saladmaker3, "[%02d:%02d:%02d.%03ld][%d][Saladmaker3][Waiting for ingredients]\n", local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, s->saladmaker3_pid);
			fflush(fp_saladmaker3);

			// αναμένει να ειδοποιηθεί από το chef
			sem_wait(&(s->saladmaker3));

			// έλεγχος για το αν χρειάζεται να γίνουν και άλλες σαλάτες
			if(s->remaining_salads <= 0){

				break;
			}
			
			/////////////////////////////////////////////////////////////////////////////
			gettimeofday(&now, NULL);
			local = localtime(&now.tv_sec);

			// γράψιμο στο αρχείο αφού έχει εξασφαλιστεί ότι γράφει μόνο
			// αυτός στο αρχείο
			sem_wait(&(s->mutex));
			fprintf(fp, "[%02d:%02d:%02d.%03ld][%d][Saladmaker3][Get ingredients]\n", local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, s->saladmaker3_pid);
			fflush(fp);
			sem_post(&(s->mutex));
			////////////////////////////////////////////////////////////////////////////////

			// γράψιμο στο αρχείο saladmaker3.txt
			fprintf(fp_saladmaker3, "[%02d:%02d:%02d.%03ld][%d][Saladmaker3][Get ingredients]\n", local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, s->saladmaker3_pid);
			fflush(fp_saladmaker3);

			// ειδοποιεί τον chef ότι πήρε τα υλικά
			sem_post(&(s->products_taken));

			////////////////////////////////////////////////////////////////////////////
			gettimeofday(&now, NULL);
			local = localtime(&now.tv_sec);

			// γράψιμο στο αρχείο αφού έχει εξασφαλιστεί ότι γράφει μόνο
			// αυτός στο αρχείο
			sem_wait(&(s->mutex));
			fprintf(fp, "[%02d:%02d:%02d.%03ld][%d][Saladmaker3][Start making salad]\n", local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, s->saladmaker3_pid);
			fflush(fp);
			sem_post(&(s->mutex));
			/////////////////////////////////////////////////////////////////////////////////

			// γράψιμο στο αρχείο saladmaker3.txt
			fprintf(fp_saladmaker3, "[%02d:%02d:%02d.%03ld][%d][Saladmaker3][Start making salad]\n", local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, s->saladmaker3_pid);
			fflush(fp_saladmaker3);

			// παρασκευή της σαλάτας
			sleep(ub - lb);
			
			////////////////////////////////////////////////////////////////////////////
			gettimeofday(&now, NULL);
			local = localtime(&now.tv_sec);

			// γράψιμο στο αρχείο αφού έχει εξασφαλιστεί ότι γράφει μόνο
			// αυτός στο αρχείο
			sem_wait(&(s->mutex));
			s->remaining_salads--;	// ενημέωρση της μεταβλητής s->remaining_salads
			fprintf(fp, "[%02d:%02d:%02d.%03ld][%d][Saladmaker3][End making salad]\n", local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, s->saladmaker3_pid);
			fflush(fp);
			sem_post(&(s->mutex));
			/////////////////////////////////////////////////////////////////////////////////

			// γράψιμο στο αρχείο saladmaker3.txt
			fprintf(fp_saladmaker3, "[%02d:%02d:%02d.%03ld][%d][Saladmaker3][End making salad]\n", local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, s->saladmaker3_pid);
			fflush(fp_saladmaker3);

			// αύξηση κατά 1 των σαλατών που έχει φτιάξει ο saladmaker3
			s->saladmaker3_made_salads++;

			// έλεγχος για το αν χρειάζεται να γίνουν και άλλες σαλάτες
			if(s->remaining_salads <= 0){
				break;
			}
		}
	}

	// κλείσιμο αρχείων
	fclose(fp);
	fclose(fp_saladmaker1);
	fclose(fp_saladmaker2);
	fclose(fp_saladmaker3);

	/* Remove segment. */
	err = shmdt((void *)s);
	if (err == -1)
		perror("Detachment.");

	return 0;
}
