///////////////////////////////////////////////////////////
//
// Υλοποίηση του ADT Priority Queue μέσω σωρού.
//
///////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "ADTPriorityQueue.h"
#include "ADTVector.h" // Η υλοποίηση του PriorityQueue χρησιμοποιεί Vector

// Ενα PriorityQueue είναι pointer σε αυτό το struct
struct priority_queue
{

	Vector vector;			   // Τα δεδομένα, σε Vector ώστε να έχουμε μεταβλητό μέγεθος χωρίς κόπο
	CompareFunc compare;	   // Η διάταξη
	DestroyFunc destroy_value; // Συνάρτηση που καταστρέφει ένα στοιχείο του vector.
};

struct priority_queue_node
{

	Pointer value1; 			// Τιμη του κομβου
	int pos;					// Θεση στο Vector
	PriorityQueue owner;
};

static int compare_p_nodes(PriorityQueueNode a, PriorityQueueNode b) {

	return a->owner->compare(a->value1, b->value1);
}

static void destroy_p_node(PriorityQueueNode node) {

	if (node->owner->destroy_value != NULL){
		node->owner->destroy_value(node->value1);
	}	

	free(node);	
}

// Βοηθητικές συναρτήσεις ////////////////////////////////////////////////////////////////////////////

// Προσοχή: στην αναπαράσταση ενός complete binary tree με πίνακα, είναι βολικό τα ids των κόμβων να
// ξεκινάνε από το 1 (ρίζα), το οποίο απλοποιεί τις φόρμουλες για εύρεση πατέρα/παιδιών. Οι θέσεις
// ενός vector όμως ξεκινάνε από το 0. Θα μπορούσαμε απλά να αφήσουμε μία θέση κενή, αλλά δεν είναι ανάγκη,
// μπορούμε απλά να αφαιρούμε 1 όταν διαβάζουμε/γράφουμε στο vector. Για απλοποίηση του κώδικα, η
// πρόσβαση στα στοιχεία του vector γίνεται από τις παρακάτω 2 βοηθητικές συναρτήσεις.

// Επιστρέφει τον κομβο με θεση node_id

static Pointer node_value(PriorityQueue pqueue, int node_id)
{

	// τα node_ids είναι 1-based, το node_id αποθηκεύεται στη θέση node_id - 1
	PriorityQueueNode node = vector_get_at(pqueue->vector, node_id - 1);
	return node;
}

// Ανταλλάσει τις τιμές των κόμβων node_id1 και node_id2

static void node_swap(PriorityQueue pqueue, int node_id1, int node_id2)
{

	// τα node_ids είναι 1-based, το node_id αποθηκεύεται στη θέση node_id - 1
	PriorityQueueNode value2 = node_value(pqueue, node_id1);
	PriorityQueueNode value3 = node_value(pqueue, node_id2);

	//ενημερωση των θεσεων των κομβων
	int temp = value2->pos;
	value2->pos = value3->pos;
	value3->pos = temp;

	vector_set_at(pqueue->vector, node_id1 - 1, value3);
	vector_set_at(pqueue->vector, node_id2 - 1, value2);
}

// Αποκαθιστά την ιδιότητα του σωρού.
// Πριν: όλοι οι κόμβοι ικανοποιούν την ιδιότητα του σωρού, εκτός από
//       τον node_id που μπορεί να είναι _μεγαλύτερος_ από τον πατέρα του.
// Μετά: όλοι οι κόμβοι ικανοποιούν την ιδιότητα του σωρού.

static void bubble_up(PriorityQueue pqueue, int node_id)
{

	// Αν φτάσαμε στη ρίζα, σταματάμε
	if (node_id == 1)
		return;

	int parent = node_id / 2; // Ο πατέρας του κόμβου. Τα node_ids είναι 1-based

	// Αν ο πατέρας έχει μικρότερη τιμή από τον κόμβο, swap και συνεχίζουμε αναδρομικά προς τα πάνω
	if (compare_p_nodes(node_value(pqueue, parent), node_value(pqueue, node_id)) < 0)
	{

		node_swap(pqueue, parent, node_id);
		bubble_up(pqueue, parent);
	}
}

// Αποκαθιστά την ιδιότητα του σωρού.
// Πριν: όλοι οι κόμβοι ικανοποιούν την ιδιότητα του σωρού, εκτός από τον
//       node_id που μπορεί να είναι _μικρότερος_ από κάποιο από τα παιδιά του.
// Μετά: όλοι οι κόμβοι ικανοποιούν την ιδιότητα του σωρού.

static void bubble_down(PriorityQueue pqueue, int node_id)
{

	// βρίσκουμε τα παιδιά του κόμβου (αν δεν υπάρχουν σταματάμε)
	int left_child = 2 * node_id;
	int right_child = left_child + 1;

	int size = pqueue_size(pqueue);
	if (left_child > size)
		return;

	// βρίσκουμε το μέγιστο από τα 2 παιδιά
	int max_child = left_child;
	if (right_child <= size && compare_p_nodes(node_value(pqueue, left_child), node_value(pqueue, right_child)) < 0)
		max_child = right_child;

	// Αν ο κόμβος είναι μικρότερος από το μέγιστο παιδί, swap και συνεχίζουμε προς τα κάτω
	if (compare_p_nodes(node_value(pqueue, node_id), node_value(pqueue, max_child)) < 0)
	{

		node_swap(pqueue, node_id, max_child);
		bubble_down(pqueue, max_child);
	}
}

// Αρχικοποιεί το σωρό από τα στοιχεία του vector values.

static void naive_heapify(PriorityQueue pqueue, Vector values)
{

	int size = vector_size(values);
	for (int i = 0; i < size; i++)
	{

		// Δέσμευση χώρου 
		PriorityQueueNode node = malloc(sizeof(*node));
		node->value1 = vector_get_at(values, i);
		node->pos = i;
		node->owner = pqueue;

		// Εισαγωγή του στοιχείου
		vector_insert_last(pqueue->vector, node);
	}


	for (int i = size / 2; i >= 1; i--)
	{

		bubble_down(pqueue, i);
	}
}

// Συναρτήσεις του ADTPriorityQueue //////////////////////////////////////////////////

PriorityQueue pqueue_create(CompareFunc compare, DestroyFunc destroy_value, Vector values)
{

	assert(compare != NULL); // LCOV_EXCL_LINE

	// Δέσμευση χώρου
	PriorityQueue pqueue = malloc(sizeof(*pqueue));
	pqueue->compare = compare;
	pqueue->destroy_value = destroy_value;
	pqueue->vector = vector_create(0, NULL);

	// Αν values != NULL, αρχικοποιούμε το σωρό.
	if (values != NULL)
		naive_heapify(pqueue, values);

	return pqueue;
}

int pqueue_size(PriorityQueue pqueue)
{

	return vector_size(pqueue->vector);
}

Pointer pqueue_max(PriorityQueue pqueue)
{

	PriorityQueueNode node = node_value(pqueue, 1);
	return node->value1; // root
}

PriorityQueueNode pqueue_insert(PriorityQueue pqueue, Pointer value)
{

	// Δέσμευση χώρου
	PriorityQueueNode node = malloc(sizeof(*node));
	node->value1 = value;
	node->pos = vector_size(pqueue->vector) + 1;
	node->owner = pqueue;

	// Προσθέτουμε τον κομβο στο τέλος το σωρού
	vector_insert_last(pqueue->vector, node);

	// Ολοι οι κόμβοι ικανοποιούν την ιδιότητα του σωρού εκτός από τον τελευταίο, που μπορεί να είναι
	// μεγαλύτερος από τον πατέρα του. Αρα μπορούμε να επαναφέρουμε την ιδιότητα του σωρού καλώντας
	// τη bubble_up γα τον τελευταίο κόμβο (του οποίου το 1-based id ισούται με το νέο μέγεθος του σωρού).
	bubble_up(pqueue, pqueue_size(pqueue));

	return node;
}

void pqueue_remove_max(PriorityQueue pqueue)
{

	int last_node = pqueue_size(pqueue);
	assert(last_node != 0); // LCOV_EXCL_LINE


	// Αντικαθιστούμε τον πρώτο κόμβο με τον τελευταίο και αφαιρούμε τον τελευταίο
	node_swap(pqueue, 1, last_node);
	vector_set_destroy_value(pqueue->vector,(DestroyFunc)destroy_p_node);
	vector_remove_last(pqueue->vector);
	vector_set_destroy_value(pqueue->vector,NULL);
	
	// Ολοι οι κόμβοι ικανοποιούν την ιδιότητα του σωρού εκτός από τη νέα ρίζα
	// που μπορεί να είναι μικρότερη από κάποιο παιδί της. Αρα μπορούμε να
	// επαναφέρουμε την ιδιότητα του σωρού καλώντας τη bubble_down για τη ρίζα.
	bubble_down(pqueue, 1);
}

DestroyFunc pqueue_set_destroy_value(PriorityQueue pqueue, DestroyFunc destroy_value)
{

	DestroyFunc old = pqueue->destroy_value;
	pqueue->destroy_value = destroy_value;
	return old;
}

void pqueue_destroy(PriorityQueue pqueue)
{

	// Αντί να κάνουμε εμείς destroy τα στοιχεία, είναι απλούστερο να
	// προσθέσουμε τη destroy_p_node στο vector ώστε να κληθεί κατά το vector_destroy.
	vector_set_destroy_value(pqueue->vector,(DestroyFunc)destroy_p_node);
	vector_destroy(pqueue->vector);

	free(pqueue);
}

//// Νέες συναρτήσεις για την εργασία 2 //////////////////////////////////////////

Pointer pqueue_node_value(PriorityQueue set, PriorityQueueNode node)
{

	return node->value1;
}

void pqueue_remove_node(PriorityQueue pqueue, PriorityQueueNode node)
{


	struct  priority_queue_node search_node = { .value1 = node->value1, .owner = pqueue };
	assert(vector_find_node(pqueue->vector, &search_node, (CompareFunc)compare_p_nodes) != NULL);

	// Αν το στοιχείο που αφαιρείται ειναι το τελευταίο του σωρού
	// κάνει απλώς destroy το τελευταίο στοιχείο
	if (node->pos == pqueue_size(pqueue))
	{

		vector_set_destroy_value(pqueue->vector,(DestroyFunc)destroy_p_node);
		vector_remove_last(pqueue->vector);
		return;
	}

	// Η θέση του κόμβου που θα αφαιρεθεί
	int id = node->pos;

	//κανω swap τον κομβο που μου δινεται με τον τελευταιο κομβο του σωρου
	int last = pqueue_size(pqueue);
	node_swap(pqueue,id,last);

	
	

	// Διαγραφή του τελευταίου κόμβου
	vector_set_destroy_value(pqueue->vector,(DestroyFunc)destroy_p_node);
	vector_remove_last(pqueue->vector);
	vector_set_destroy_value(pqueue->vector,NULL);
	
	// Αν δεν είναι η ρίζα και η τιμή του κόμβου είναι μεγαλύτερη απο αυτη του πατερα του
	// καλεί την bubble_up για αποκατάσταση του σωρού
	PriorityQueueNode node_pos_id = node_value(pqueue,id);
	if (id != 1)
	{

		PriorityQueueNode parent = node_value(pqueue, id / 2);
		if (compare_p_nodes(node_pos_id, parent) > 0)
		{

			bubble_up(pqueue, node_pos_id->pos);
		}
	}
	// Αλλιώς αν δεν είναι φύλλα
	else if (2 * node_pos_id->pos <= vector_size(pqueue->vector))
	{

		// Καλεί την bubble_down για αποκατάσταση του σωρού
		bubble_down(pqueue, node_pos_id->pos);
	}
}

void pqueue_update_order(PriorityQueue pqueue, PriorityQueueNode node)
{

	// Αν δεν είναι η ρίζα και η τιμή του κόμβου είναι μεγαλύτερη απο αυτη του πατερα του
	// καλεί την bubble_up για αποκατάσταση του σωρού
	if (node->pos != 1)
	{

		PriorityQueueNode parent = node_value(pqueue, node->pos / 2);
		if (compare_p_nodes(node, parent) > 0)
		{

			bubble_up(pqueue, node->pos);
		}
	}
	// Αλλιώς αν δεν είναι φύλλα
	else if (2 * node->pos <= vector_size(pqueue->vector))
	{

		bubble_down(pqueue, node->pos);
	}
}
