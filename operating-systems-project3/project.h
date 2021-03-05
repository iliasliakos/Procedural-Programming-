struct shared_memory{

	sem_t saladmaker1;
	sem_t saladmaker2;
	sem_t saladmaker3;

	sem_t products_taken;

	sem_t mutex;

	int remaining_salads;

	int saladmaker1_made_salads;
	int saladmaker2_made_salads;
	int saladmaker3_made_salads;

	int saladmaker1_pid;
	int saladmaker2_pid;
	int saladmaker3_pid;
};