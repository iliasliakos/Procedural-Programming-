///////////////////////////////////////////////////////////
//
// Υλοποίηση του ADT BList μέσω διπλά συνδεδεμένης λίστας.
//
///////////////////////////////////////////////////////////

#include <stdlib.h>
#include <assert.h>

#include "ADTBList.h"

// Ενα BList είναι pointer σε αυτό το struct
struct blist
{
	BListNode dummy;		   // χρησιμοποιούμε dummy κόμβο, ώστε ακόμα και η κενή λίστα να έχει έναν κόμβο.
	BListNode last;			   // δείκτης στον τελευταίο κόμβο, ή στον dummy (αν η λίστα είναι κενή)
	int size;				   // μέγεθος, ώστε η list_size να είναι Ο(1)
	DestroyFunc destroy_value; // Συνάρτηση που καταστρέφει ένα στοιχείο της λίστας.
};

struct blist_node
{
	BListNode previous; // Δεικτης στον προηγουμενο
	BListNode next;		// Δείκτης στον επόμενο
	Pointer value;		// Η τιμή που αποθηκεύουμε στον κόμβο
};

BList blist_create(DestroyFunc destroy_value)
{
	// Πρώτα δημιουργούμε το stuct
	BList blist = malloc(sizeof(*blist));
	blist->size = 0;
	blist->destroy_value = destroy_value;

	// Χρησιμοποιούμε dummy κόμβο, ώστε ακόμα και μια άδεια λίστα να έχει ένα κόμβο

	blist->dummy = malloc(sizeof(*blist->dummy));
	blist->dummy->next = NULL;	   // άδεια λίστα, ο dummy δεν έχει επόμενο
	//blist->dummy->previous = NULL; //o dummy δεν εχει προηγουμενο

	// Σε μια κενή λίστα, τελευταίος κόμβος είναι επίσης ο dummy
	blist->last = blist->dummy;

	return blist;
}

int blist_size(BList blist)
{
	return blist->size;
}

void blist_insert(BList blist, BListNode node, Pointer value)
{
	// Δημιουργία του νέου κόμβου
	BListNode new = malloc(sizeof(*new));
	new->value = value;
	// Αν το node είναι NULL  εισάγουμε τον κομβο στο τελος της λιστας
	if (node == NULL)
	{
		blist->last->next = new;
		new->next = NULL;
		new->previous = blist->last;
		//ενημερωση του last
		blist->last = new;
	}
	else
	{
		//συνδεση του new αναμεσα απο τον node->previous και τον node
		node->previous->next = new;
		new->previous = node->previous;
		new->next = node;
		node->previous = new;
	}
	//ενημερωση του last
	if (blist->size == 0)
	{
		blist->last = new;
	}

	// Ενημέρωση του size
	blist->size++;
}

void blist_remove(BList blist, BListNode node)
{
	//ο κομβος πρεπει να υπαρχει
	assert(node != NULL);

	BListNode previous = node->previous;
	if (blist->destroy_value != NULL)
		blist->destroy_value(node->value);

	// Σύνδεση του previous με τον επόμενο του node
	previous->next = node->next;		// πριν το free!

	//free(node);

	// Ενημέρωση των size & last
	blist->size--;
	if (blist->last == node)
		blist->last = previous;

	free(node);
}

Pointer blist_find(BList blist, Pointer value, CompareFunc compare)
{
	BListNode node = blist_find_node(blist, value, compare);
	return node == NULL ? NULL : node->value;
}

DestroyFunc blist_set_destroy_value(BList blist, DestroyFunc destroy_value)
{
	DestroyFunc old = blist->destroy_value;
	blist->destroy_value = destroy_value;
	return old;
}

void blist_destroy(BList blist)
{
	// Διασχίζουμε όλη τη λίστα και κάνουμε free όλους τους κόμβους,
	// συμπεριλαμβανομένου και του dummy

	BListNode node = blist->dummy;
	while (node != NULL)
	{
		BListNode next = node->next;
		// Καλούμε τη destroy_value, αν υπάρχει
		if (node != blist->dummy && blist->destroy_value != NULL)
			blist->destroy_value(node->value);
			
			

		free(node);
		node = next;
	}

	// Τέλος free το ίδιο το struct
	free(blist);
}

// Διάσχιση της λίστας /////////////////////////////////////////////

BListNode blist_first(BList blist)
{
	// Ο πρώτος κόμβος είναι ο επόμενος του dummy.
	//
	return blist->dummy->next;
}

BListNode blist_last(BList blist)
{
	if (blist->last == blist->dummy)
		return BLIST_EOF; // κενή λίστα
	else
		return blist->last;
}

BListNode blist_next(BList blist, BListNode node)
{
	assert(node != NULL);
	return node->next;
}

BListNode blist_previous(BList blist, BListNode node)
{
	assert(node != NULL);
	return node->previous;
}

Pointer blist_node_value(BList blist, BListNode node)
{
	assert(node != NULL);
	return node->value;
}

BListNode blist_find_node(BList blist, Pointer value, CompareFunc compare)
{
	// διάσχιση όλης της λίστας, καλούμε την compare μέχρι να επιστρέψει 0
	//
	for (BListNode node = blist->dummy->next; node != NULL; node = node->next)
		if (compare(value, node->value) == 0)
			return node; // βρέθηκε

	return NULL; // δεν υπάρχει
}