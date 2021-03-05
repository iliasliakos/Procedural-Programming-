#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "project.h"

#define SEGMENTSIZE sizeof(sem_t)
#define SEGMENTPERM 0666

int main(int argc, char **argv)
{

	struct shared_memory *s;
	struct timeval now;
	struct tm *local;
	int retval, err;
	int id;
	FILE *fp;
	int chef_pid;

	// το pid του chef
	chef_pid = getpid();

	// ο αριθμός των σαλατών που πρέπει να γίνουν
	int numofSlds = atoi(argv[2]);
	printf("numofSlds is %d", numofSlds);

	// η ώρα ξεκούρασης του chef
	int mantime = atoi(argv[4]);
	printf("mantime is %d\n", mantime);

	/* Make shared memory segment. */
	id = shmget(IPC_PRIVATE, sizeof(struct shared_memory), SEGMENTPERM);
	if (id == -1)
		perror("Creation");
	else
		printf("Allocated %d\n", id);

	/* Attach the segment. */
	s = (struct shared_memory *)shmat(id, (void *)0, 1);
	if (s == (void *)-1)
	{
		perror("Attachment.");
		exit(2);
	}

	// αρχικοποίηση της μεταβλητής για τις σαλάτες που έχουν απομείνει
	s->remaining_salads = numofSlds;

	// αρχικοποίηση των μεταβλητών που μετράνε
	// πόσες σαλάτες έχει φτιάξει ένας saladmaker
	s->saladmaker1_made_salads = 0;
	s->saladmaker2_made_salads = 0;
	s->saladmaker3_made_salads = 0;

	// αρχικοποίηση των pid των saladmakers
	s->saladmaker1_pid = -10;
	s->saladmaker2_pid = -10;
	s->saladmaker3_pid = -10;

	/////////////////////////////////////////////////////////////////////////////////////////////
	// semaphore για ctritical problem solution
	////////////////////////////////////////////////////////////////////////////////////////////

	/* Initialize the semaphore. */
	retval = sem_init(&(s->mutex), 1, 1);
	if (retval != 0)
	{
		perror("Couldn't initialize.");
		exit(3);
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// semaphore για να ειδοποιείται ο chef ότι ο saladmaker πήρε τα υλικά
	////////////////////////////////////////////////////////////////////////////////

	/* Initialize the semaphore. */
	retval = sem_init(&(s->products_taken), 1, 0);
	if (retval != 0)
	{
		perror("Couldn't initialize.");
		exit(3);
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// semaphore for first saladmaker
	////////////////////////////////////////////////////////////////////////////////////

	/* Initialize the semaphore. */
	retval = sem_init(&(s->saladmaker1), 1, 0);
	if (retval != 0)
	{
		perror("Couldn't initialize.");
		exit(3);
	}

	////////////////////////////////////////////////////////////////
	//          semaphore for second saladmaker
	//////////////////////////////////////////////////////////////////////////////////////

	/* Initialize the semaphore. */
	retval = sem_init(&(s->saladmaker2), 1, 0);
	if (retval != 0)
	{
		perror("Couldn't initialize.");
		exit(3);
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	//        semaphore for third saladmaker
	///////////////////////////////////////////////////////////////////////////////////////

	/* Initialize the semaphore. */
	retval = sem_init(&(s->saladmaker3), 1, 0);
	if (retval != 0)
	{
		perror("Couldn't initialize.");
		exit(3);
	}

	////////////////////////////////////////////////////////////////////////////////////////
	//			Ενιαίο log file
	/////////////////////////////////////////////////////////////////////////

	fp = fopen(argv[5], "a");
	if (fp == NULL)
	{
		printf("Error: open() failed\n");
	}

	// επιλογή υλικών
	///////////////////////////////////////////////////////////////////////////
	//				0 == ντομάτα
	//				1 == πιπεριά
	//				2 == κρεμμύδι
	// 				ο πρώτος saladmaker έχει πάντα ντομάτα
	//				ο δεύτερος saladmaker έχει πάντα πιπεριά
	//				ο τρίτος saladmaker έχει πάντα κρεμμύδι
	///////////////////////////////////////////////////////////////////////////

	int previous_product1 = -2;
	int previous_product2 = -2;

	int y = 0; // μετράει τις παραγγελίες που έχει δώσει ο chef

	while (y < numofSlds)
	{

		int product1 = -1;
		int product2 = -1;
		int random;

		srand(time(0));
		while (1)
		{

			for (int i = 0; i < 2; i++)
			{

				random = rand() % 3; // διάλεξε έναν αριθμό από το 0 έως το 2

				if (i == 0)
				{

					// αποθήκευση του πρώτου υλικού
					product1 = random;
				}
				else
				{

					// αποθήκευση του δεύτερου υλικού
					product2 = random;
				}
			}

			// αν δεν έχει διαλέξει 2 φορες το ίδιο προϊόν
			if (product2 != product1)
			{
				// αν δεν έχει διαλέξει τα ίδια προϊόντα για δεύτερη φορά
				if (previous_product1 != product1 || previous_product2 != product2)
				{

					if (previous_product1 != product2 || previous_product2 != product1)
					{

						previous_product1 = product1;
						previous_product2 = product2;

						// προχωράμε στην ανάθεση της παραγγελίας
						break;
					}
				}
			}
		}

		printf(" product1 = %d  product2 = %d\n", product1, product2);

		gettimeofday(&now, NULL);
		local = localtime(&now.tv_sec);

		// αν έχει διαλέξει ντομάτα και πιπεριά
		if ((product1 == 0 && product2 == 1) || (product2 == 0 && product1 == 1))
		{

			////////////////////////////////////////////////////////////////////////////
			// γράψιμο στο αρχείο αφού έχει εξασφαλιστεί ότι γράφει μόνο
			// αυτός στο αρχείο
			sem_wait(&(s->mutex));
			fprintf(fp, "[%02d:%02d:%02d.%03ld][%d][Chef][Selecting ingredients ntomata piperia]\n", local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, chef_pid);
			fflush(fp);
			sem_post(&(s->mutex));
			////////////////////////////////////////////////////////////////////////////////////

			// ειδοποίηση του saladmaker3
			sem_post(&(s->saladmaker3));

			///////////////////////////////////////////////////////////////////////////
			gettimeofday(&now, NULL);
			local = localtime(&now.tv_sec);

			// γράψιμο στο αρχείο αφού έχει εξασφαλιστεί ότι γράφει μόνο
			// αυτός στο αρχείο
			sem_wait(&(s->mutex));
			fprintf(fp, "[%02d:%02d:%02d.%03ld][%d][Chef][Notify saladmaker #3]\n", local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, chef_pid);
			fflush(fp);
			sem_post(&(s->mutex));
			////////////////////////////////////////////////////////////////////////////////////

			// αναμονή μέχρι να παραλάβει ο saladmaker τα υλικά
			sem_wait(&(s->products_taken));
			y++; // αύξηση των παραγγελιών

			////////////////////////////////////////////////////////////////////////////
			gettimeofday(&now, NULL);
			local = localtime(&now.tv_sec);

			// γράψιμο στο αρχείο αφού έχει εξασφαλιστεί ότι γράφει μόνο
			// αυτός στο αρχείο
			sem_wait(&(s->mutex));
			fprintf(fp, "[%02d:%02d:%02d.%03ld][%d][Chef][Man time for resting]\n", local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, chef_pid);
			fflush(fp);
			sem_post(&(s->mutex));
			////////////////////////////////////////////////////////////////////////////////////

			// ξεκούραση του chef
			sleep(mantime);
		}
		// αν έχει διαλέξει ντομάτα και κρεμμύδι
		else if ((product1 == 0 && product2 == 2) || (product2 == 0 && product1 == 2))
		{

			////////////////////////////////////////////////////////////////////////////////////////
			// γράψιμο στο αρχείο αφού έχει εξασφαλιστεί ότι γράφει μόνο
			// αυτός στο αρχείο
			sem_wait(&(s->mutex));
			fprintf(fp, "[%02d:%02d:%02d.%03ld][%d][Chef][Selecting ingredients ntomata kremidi]\n", local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, chef_pid);
			fflush(fp);
			sem_post(&(s->mutex));
			/////////////////////////////////////////////////////////////

			// ειδοποίηση του saladmaker2
			sem_post(&(s->saladmaker2));

			////////////////////////////////////////////////////////////////////////////

			gettimeofday(&now, NULL);
			local = localtime(&now.tv_sec);

			// γράψιμο στο αρχείο αφού έχει εξασφαλιστεί ότι γράφει μόνο
			// αυτός στο αρχείο
			sem_wait(&(s->mutex));
			fprintf(fp, "[%02d:%02d:%02d.%03ld][%d][Chef][Notify saladmaker #2]\n", local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, chef_pid);
			fflush(fp);
			sem_post(&(s->mutex));
			////////////////////////////////////////////////////////////////////////////////////

			// αναμονή μέχρι να παραλάβει ο saladmaker τα υλικά
			sem_wait(&(s->products_taken));
			y++; // αύξηση των παραγγελιών

			////////////////////////////////////////////////////////////////////////////

			gettimeofday(&now, NULL);
			local = localtime(&now.tv_sec);

			// γράψιμο στο αρχείο αφού έχει εξασφαλιστεί ότι γράφει μόνο
			// αυτός στο αρχείο
			sem_wait(&(s->mutex));
			fprintf(fp, "[%02d:%02d:%02d.%03ld][%d][Chef][Man time for resting]\n", local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, chef_pid);
			fflush(fp);
			sem_post(&(s->mutex));
			////////////////////////////////////////////////////////////////////////////////////

			// ξεκούραση του chef
			sleep(mantime);
		}
		// αν έχει διαλέξει πιπεριά κρεμμύδι
		else
		{

			///////////////////////////////////////////////////////////////////////

			// γράψιμο στο αρχείο αφού έχει εξασφαλιστεί ότι γράφει μόνο
			// αυτός στο αρχείο
			sem_wait(&(s->mutex));
			fprintf(fp, "[%02d:%02d:%02d.%03ld][%d][Chef][Selecting ingredients piperia kremidi]\n", local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, chef_pid);
			fflush(fp);
			sem_post(&(s->mutex));
			//////////////////////////////////////////////////////////////////////

			// ειδοποίηση του saladmaker1
			sem_post(&(s->saladmaker1));

			////////////////////////////////////////////////////////////////////////////

			gettimeofday(&now, NULL);
			local = localtime(&now.tv_sec);

			// γράψιμο στο αρχείο αφού έχει εξασφαλιστεί ότι γράφει μόνο
			// αυτός στο αρχείο
			sem_wait(&(s->mutex));
			fprintf(fp, "[%02d:%02d:%02d.%03ld][%d][Chef][Notify saladmaker #1]\n", local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, chef_pid);
			fflush(fp);
			sem_post(&(s->mutex));
			////////////////////////////////////////////////////////////////////////////////////

			// αναμονή μέχρι να παραλάβει ο saladmaker τα υλικά
			sem_wait(&(s->products_taken));
			y++; // αύξηση των παραγγελιών

			////////////////////////////////////////////////////////////////////////////

			gettimeofday(&now, NULL);
			local = localtime(&now.tv_sec);

			// γράψιμο στο αρχείο αφού έχει εξασφαλιστεί ότι γράφει μόνο
			// αυτός στο αρχείο
			sem_wait(&(s->mutex));
			fprintf(fp, "[%02d:%02d:%02d.%03ld][%d][Chef][Man time for resting]\n", local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, chef_pid);
			fflush(fp);
			sem_post(&(s->mutex));
			////////////////////////////////////////////////////////////////////////////////////

			// ξεκούραση του chef
			sleep(mantime);
		}

		// αν έχουν δοθεί όσες παραγγελίες χρειάζεται
		if (y >= numofSlds)
		{

			// ο chef περιμένει μέχρι οι saladmakers να φτίαξου τις παραγγελίες
			while (1)
			{

				// αν οι παραγγελίες έχουν φτιαχτεί
				if (s->remaining_salads <= 0)
				{

					// ο chef ειδοποιεί για τελευταία φορά
					// τους saladmakers για να τερματίσουν
					// όσοι δεν έχουν ήδη τερματίσει
					sem_post(&(s->saladmaker1));
					sem_post(&(s->saladmaker2));
					sem_post(&(s->saladmaker3));

					break;
				}
			}
		}
	}

	printf("total salads = %d\n", -s->remaining_salads + numofSlds);
	printf("#salads of salad_maker1 [%d] : [%d]\n", s->saladmaker1_pid, s->saladmaker1_made_salads);
	printf("#salads of salad_maker2 [%d] : [%d]\n", s->saladmaker2_pid, s->saladmaker2_made_salads);
	printf("#salads of salad_maker3 [%d] : [%d]\n", s->saladmaker3_pid, s->saladmaker3_made_salads);

	/////////////////////////////////////////////////////////////////////
	// destroy τα semaphores
	sem_destroy(&(s->saladmaker1));
	sem_destroy(&(s->saladmaker2));
	sem_destroy(&(s->saladmaker3));
	sem_destroy(&(s->products_taken));
	sem_destroy(&(s->mutex));
	///////////////////////////////////////////////////////////

	/* Remove segment. */
	err = shmctl(id, IPC_RMID, 0);
	if (err == -1)
		perror("Removal.");
	else
		printf("Removed. %d\n", err);

	// κλείσιμο του αρχείου
	fclose(fp);

	// άνοιγμα αρχείου για διάβασμα
	fp = fopen(argv[5], "r");
	if (fp == NULL)
	{
		printf("Error: open() failed\n");
	}

	///////////////////////////////////////////////////////////////////////
	// εύρεση κοινών διαστημάτων
	char line[200];
	const char d[2] = "[";
	char *token_end;
	char *token_start;

	int flag = 0;

	while (1)
	{

		if (fgets(line, 200, fp) == NULL)
		{
			break;
		}

		if (strstr(line, "Start") != NULL)
		{

			flag++;
			if (flag == 2)
			{

				token_start = strtok(line, d);
				printf("[%s , ", token_start);
			}
		}
		else if (strstr(line, "End") != NULL)
		{

			flag--;

			if (flag == 1)
			{

				token_end = strtok(line, d);
				printf("[%s\n", token_end);

				token_end = NULL;
				token_start = NULL;
			}
		}
	}
	///////////////////////////////////////////////////////////////////

	// κλείσιμο αρχείου
	fclose(fp);

	return 0;
}
