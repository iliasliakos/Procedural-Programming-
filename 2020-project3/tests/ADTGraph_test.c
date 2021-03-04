//////////////////////////////////////////////////////////////////
//
// Unit tests για τον ADT Graph.
// Οποιαδήποτε υλοποίηση οφείλει να περνάει όλα τα tests.
//
//////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include "acutest.h"			// Απλή βιβλιοθήκη για unit testing

#include "ADTGraph.h"


// Δημιουργούμε μια ειδική compare συνάρτηση
int compare_ints(Pointer a, Pointer b) {
	return *(int*)a - *(int*)b;
}

void test_create(void) {

	// Δημιουργούμε μια κενή λίστα (χωρίς αυτόματο free)
	Graph graph = graph_create(compare_ints, free);
	graph_set_hash_function(graph, hash_int);

	// Ελέγχουμε ότι δεν απέτυχε η malloc στην λίστα, και ότι
	// αρχικοποιείται με μέγεθος 0 (δηλαδή χωρίς κόμβους)
	TEST_ASSERT(graph != NULL);
	TEST_ASSERT(graph_size(graph) == 0);

	graph_destroy(graph);
}

// Επιστρέφει έναν ακέραιο σε νέα μνήμη με τιμή value
int* create_int(int value) {
	int* p = malloc(sizeof(int));
	*p = value;
	return p;
}

// Βοηθητική συνάρτηση, κάνει insert μια κορυφή και ελέγχει αν έγινε η εισαγωγή
void insert_vertex_and_test(Graph graph, Pointer vertex) {

	graph_insert_vertex(graph, vertex);

	// Βρίσκει τη λίστα με τις κορυφές του γράφου
	// και ελέγχει αν η κορυφή που μπήκε υπάρχει στη λίστα
	List list_of_vertices = graph_get_vertices(graph);
	TEST_ASSERT(list_find_node(list_of_vertices, vertex, compare_ints) != NULL);

	// Destroy τη λίστα
	list_destroy(list_of_vertices);
}

// Βοηθητική συνάρτηση, κάνει insert μια ακμή και ελέγχει αν έγινε η εισαγωγή
void insert_edge_and_test(Graph graph, Pointer vertex1, Pointer vertex2, uint weight){

	graph_insert_edge(graph, vertex1, vertex2, weight);

	// Βρίσκει τη λίστα με τους γείτονες της κορυφης vertex1
	// και ελέγχει αν η κορυφή vertex2 υπάρχει στη λίστα
	List adjacent_list_of_vertex1 = graph_get_adjacent(graph, vertex1);
	TEST_ASSERT(list_find_node(adjacent_list_of_vertex1, vertex2, compare_ints) != NULL);

	// Destroy τη λίστα
	list_destroy(adjacent_list_of_vertex1);

	// Ελέγχει αν ενημερώθηκε το βάρος από τη vertex1 στη vertex2 
	TEST_ASSERT(graph_get_weight(graph, vertex1, vertex2) == weight);
}

void remove_edge_and_test(Graph graph, Pointer vertex1, Pointer vertex2){

	graph_remove_edge(graph, vertex1, vertex2);

	// Βρίσκει τη λίστα με τους γείτονες της vertex1
	// και ελέγχει ότι η vertex2
	// δεν ανοίκει στη λίστα
	List list = graph_get_adjacent(graph, vertex1);
	TEST_ASSERT(list_find_node(list, vertex2, compare_ints) == LIST_EOF);

	// Destroy τη λίστα
	list_destroy(list);
}

// Βοηθητική συνάρτηση για το ανακάτεμα του πίνακα τιμών
void shuffle(int* array[], int n) {
	for (int i = 0; i < n; i++) {
		int j = i + rand() / (RAND_MAX / (n - i) + 1);
		int* t = array[j];
		array[j] = array[i];
		array[i] = t;
	}
}

void test_insert(void) {

	Graph graph = graph_create(compare_ints, free);
	graph_set_hash_function(graph, hash_int);

	int N = 1000;
	int** key_array = malloc(N * sizeof(*key_array));

	for (int i = 0; i < N; i++) {

		key_array[i] = create_int(i);
	}

	// Ανακατεύουμε το key_array ώστε να υπάρχει ομοιόμορφη εισαγωγή τιμών
	shuffle(key_array, N);

	/////////////////////////////////////////////////////////////////////////////
	// Δοκιμάζουμε την graph_insert_vertex εισάγοντας κάθε φορά νέους κόμβους
	////////////////////////////////////////////////////////////////////////////////

	for (int i = 0; i < N; i++) {

		// Εισαγωγή, δοκιμή και έλεγχος ότι ενημερώθηκε το size
		insert_vertex_and_test(graph, key_array[i]);
		TEST_ASSERT(graph_size(graph) == (i + 1)); 
	}

	////////////////////////////////////////////////////////////////////
	// Δοκιμάζουμε την graph_insert_edge 
	////////////////////////////////////////////////////////////////////

	// Εισαγωγή όλων των κορυφών του γράφου ως γείτονες της πρώτης κορυφής
	for (int i = 0; i < N; i++) {

		if(i != 0){

			insert_edge_and_test(graph, key_array[0], key_array[i], (uint)i);
		}	 
	}

	// Εισαγωγή όλως των κορυφών του γράφου ως γείτονες της δεύτερης κορυφής
	for (int i = 0; i < N; i++) {

		if(i != 1){

			insert_edge_and_test(graph, key_array[0], key_array[i], (uint)i);
		}	 
	}

	graph_destroy(graph);
	free(key_array);
}

void test_remove(void) {

	//////////////////////////////////////////////////
	// Έλεγχος της graph_remove_vertex
	//////////////////////////////////////////////////

	Graph graph = graph_create(compare_ints, free);
	graph_set_hash_function(graph, hash_int);

	int N = 1000;
	int** key_array = malloc(N * sizeof(*key_array));

	for (int i = 0; i < N; i++) {

		key_array[i] = create_int(i);
	}

	// Ανακατεύουμε το key_array ώστε να υπάρχει ομοιόμορφη εισαγωγή τιμών
	shuffle(key_array, N);

	int y = 1;        	
	int size = 0;     // Μετράει το size του γράφου όταν προσθεθεί μια κορυφή	
	for (int i = 0; i < N; i++) {

		insert_vertex_and_test(graph, key_array[i]);
		size++;

		// Ανά τακτά χρονικά διαστήματα διαγράφουμε κάποια κορυφή που μόλις βάλαμε
		if (i % (N / 20) == 0){

			// Διαγραφεί κορυφής και test ότι το size ενημερώθηκε
			graph_remove_vertex(graph, key_array[i]);
			TEST_ASSERT(graph_size(graph) == size - y);
			y++;
		}	
	}
	
	graph_destroy(graph);
	free(key_array);

	///////////////////////////////////////////////////
	// Έλεγχος της graph_remove_edge
	///////////////////////////////////////////////////

	Graph graph1 = graph_create(compare_ints, free);
	graph_set_hash_function(graph1, hash_int);

	int N1 = 1000;
	int** key_array1 = malloc(N1 * sizeof(*key_array1));

	

	for (int i = 0; i < N1; i++) {

		key_array1[i] = create_int(i);
	}

	// Ανακατεύουμε το key_array ώστε να υπάρχει ομοιόμορφη εισαγωγή τιμών
	shuffle(key_array1, N1);

	// Εισαγωγή κορυφών στο γράφο
	for (int i = 0; i < N1; i++) {

		//key_array1[i] = create_int(i);
		graph_insert_vertex(graph1, key_array1[i]);
	}

	for (int i = 0; i < N1; i++) {

		insert_edge_and_test(graph1, key_array1[0], key_array1[i], (uint)i);
		

		// Ανά τακτά χρονικά διαστήματα διαγράφουμε κάποια ακμή που μόλις βάλαμε
		if (i % (N1 / 20) == 0){

			// Διαγραφεί ακμής 
			remove_edge_and_test(graph1, key_array1[0], key_array1[i]);
		}	
	}

	
	graph_destroy(graph1);
	free(key_array1);
}

void test_shortest_path(void){

	////////////////////////////////////
	// TEST 1
	////////////////////////////////////

	Graph graph = graph_create(compare_ints, free);
	graph_set_hash_function(graph, hash_int);

	int** vertex = malloc(7 * sizeof(*vertex));

	// Εισαγωγή κορυφών στο γράφο
	for (int i = 0; i < 7; i++) {

		vertex[i] = create_int(i);
		if(i != 0){

			insert_vertex_and_test(graph, vertex[i]);
		}	
	}
	
	// Εισαγωγή ακμών στο γράφο
	// Αναπαράσταση του γράφου https://k08.chatzi.org/slides/weighted-graphs/#/6

	insert_edge_and_test(graph, vertex[1], vertex[2], 3);
	insert_edge_and_test(graph, vertex[1], vertex[6], 5);
	insert_edge_and_test(graph, vertex[2], vertex[6], 10);
	insert_edge_and_test(graph, vertex[2], vertex[3], 7);
	insert_edge_and_test(graph, vertex[3], vertex[4], 5);
	insert_edge_and_test(graph, vertex[3], vertex[5], 1);
	insert_edge_and_test(graph, vertex[4], vertex[5], 6);
	insert_edge_and_test(graph, vertex[5], vertex[6], 7);
	insert_edge_and_test(graph, vertex[6], vertex[3], 8);
	insert_edge_and_test(graph, vertex[6], vertex[4], 2);
	

	List list = graph_shortest_path(graph, vertex[1], vertex[5]);
	
	int array[4] = {1, 2, 3, 5};       // To βέλτιστο μονοπάτι

	TEST_ASSERT(list_size(list) == 4);

	int i = 0;
	// Διάσχιση της λίστας και έλεγχος 
	// αν οι τιμές της λίστα είναι ισοδύναμες με του πίνακα
	// που περιέχει το βέλτιστο μονοπάτι
    for (ListNode node = list_first(list);
        node != LIST_EOF;
        node = list_next(list, node))
    {

		int* value = list_node_value(list, node);
		TEST_ASSERT(*value == array[i]);
		i++;
	}

	// Destroy τη λίστα
	list_destroy(list);
	// Destroy το γράφο
	graph_destroy(graph);
	// Free την κορυφή που δεν χρησιμοποιήθηκε
	free(vertex[0]);

	free(vertex);

	//////////////////////////////////
	// TEST 2
	//////////////////////////////////

	Graph graph2 = graph_create(compare_ints, free);
	graph_set_hash_function(graph2, hash_int);

	int** vertex2 = malloc(7 * sizeof(*vertex2));

	// Εισαγωγή κορυφών στο γράφο
	for (int i = 0; i < 7; i++) {

		vertex2[i] = create_int(i);
		insert_vertex_and_test(graph2, vertex2[i]);
	}
	
	// Εισαγωγή ακμών στο γράφο
	// Αναπαράσταση του γράφου 
	///////////////////////////////////////////////
	// (1) --3--* (2)
	//  |          |
	//  5          50
	//  |          |
	//  *          *
	// (6) --8--* (3) --5--* (4)
	//             |		  |
	//             6          60
	//             |          |
	//             *          *
	//            (5) --1--* (0) 
	////////////////////////////////////////////

	insert_edge_and_test(graph2, vertex2[1], vertex2[2], 3);
	insert_edge_and_test(graph2, vertex2[1], vertex2[6], 5);
	insert_edge_and_test(graph2, vertex2[2], vertex2[3], 50);
	insert_edge_and_test(graph2, vertex2[3], vertex2[4], 5);
	insert_edge_and_test(graph2, vertex2[3], vertex2[5], 6);
	insert_edge_and_test(graph2, vertex2[4], vertex2[0], 60);
	insert_edge_and_test(graph2, vertex2[5], vertex2[0], 1);
	insert_edge_and_test(graph2, vertex2[6], vertex2[3], 8);
	
	
	List list2 = graph_shortest_path(graph2, vertex2[1], vertex2[0]);
	
	int array2[5] = {1, 6, 3, 5, 0};       // To βέλτιστο μονοπάτι

	TEST_ASSERT(list_size(list2) == 5);

	int y = 0;
	// Διάσχιση της λίστας και έλεγχος 
	// αν οι τιμές της λίστα είναι ισοδύναμες με του πίνακα
	// που περιέχει το βέλτιστο μονοπάτι
    for (ListNode node = list_first(list2);
        node != LIST_EOF;
        node = list_next(list2, node))
    {

		int* value = list_node_value(list2, node);
		TEST_ASSERT(*value == array2[y]);
		y++;
	}

	// Destroy τη λίστα
	list_destroy(list2);
	// Destroy το γράφο
	graph_destroy(graph2);
	
	free(vertex2);
}

// Λίστα με όλα τα tests προς εκτέλεση
TEST_LIST = {
	// { "create", test_create },
	{ "graph_create", test_create },
	{ "graph_insert", test_insert },
	{ "graph_remove", test_remove },
	{ "graph_shortest_path", test_shortest_path},
	{ NULL, NULL } // τερματίζουμε τη λίστα με NULL
}; 