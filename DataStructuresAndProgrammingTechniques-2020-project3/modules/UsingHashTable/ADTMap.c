/////////////////////////////////////////////////////////////////////////////
//
// Υλοποίηση του ADT Map μέσω Hash Table με separate chaining
//
/////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>

#include <ADTBList.h>
#include "ADTMap.h"

// Το μέγεθος του Hash Table ιδανικά θέλουμε να είναι πρώτος αριθμός σύμφωνα με την θεωρία.
// Η παρακάτω λίστα περιέχει πρώτους οι οποίοι έχουν αποδεδιγμένα καλή συμπεριφορά ως μεγέθη.
// Κάθε re-hash θα γίνεται βάσει αυτής της λίστας. Αν χρειάζονται παραπάνω απο 1610612741 στοχεία, τότε σε καθε rehash διπλασιάζουμε το μέγεθος.
int prime_sizes[] = {53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317, 196613, 393241,
					 786433, 1572869, 3145739, 6291469, 12582917, 25165843, 50331653, 100663319, 201326611, 402653189, 805306457, 1610612741};

// Χρησιμοποιούμε separate chaining, οπότε σύμφωνα με την θεωρία, πρέπει πάντα να διατηρούμε
// τον load factor του  hash table μικρότερο ή ίσο του 0.9, για να έχουμε αποδoτικές πράξεις
#define MAX_LOAD_FACTOR 0.9

// Δομή που κάθε κόμβος της λίστας θα περιέχει δείκτη σε αυτή
struct map_node
{

	Pointer key;
	Pointer value;
	int array_index; // Σε ποιό index του array είναι το στοιχείο
	Map owner;
};

// Δομή του Map (περιέχει όλες τις πληροφορίες που χρεαζόμαστε για το HashTable)
struct map
{

	BList *array; // Το hash table που περιεχει δεικτες σε λιστες
	int capacity;
	int size;
	CompareFunc compare;
	HashFunc hash_function;
	DestroyFunc destroy_key;
	DestroyFunc destroy_value;
};

static int compare_map_nodes(MapNode a, MapNode b)
{

	return a->owner->compare(a->key, b->key);
}

// Συνάρτηση που καταστρέφει ένα map node
static void destroy_map_node(MapNode node)
{

	if (node->owner->destroy_key != NULL)
		node->owner->destroy_key(node->key);

	if (node->owner->destroy_value != NULL)
		node->owner->destroy_value(node->value);

	free(node);
}

Map map_create(CompareFunc compare, DestroyFunc destroy_key, DestroyFunc destroy_value)
{
	// Δεσμεύουμε κατάλληλα τον χώρο που χρειαζόμαστε για το hash table
	Map map = malloc(sizeof(*map));
	map->capacity = prime_sizes[0];
	map->array = malloc(map->capacity * sizeof(BList));

	// Αρχικοποιούμε τον πινακα με κενές λίστες
	for (int i = 0; i < map->capacity; i++)
	{
		map->array[i] = blist_create((DestroyFunc)destroy_map_node);
	}

	map->size = 0;
	map->compare = compare;
	map->destroy_key = destroy_key;
	map->destroy_value = destroy_value;

	return map;
}

// Επιστρέφει τον αριθμό των entries του map σε μία χρονική στιγμή.
int map_size(Map map)
{
	return map->size;
}

// Συνάρτηση για την επέκταση του Hash Table σε περίπτωση που ο load factor μεγαλώσει πολύ.
static void rehash(Map map)
{
	// Αποθήκευση των παλιών δεδομένων
	int old_capacity = map->capacity;
	BList *old_array = map->array;

	// Βρίσκουμε τη νέα χωρητικότητα, διασχίζοντας τη λίστα των πρώτων ώστε να βρούμε τον επόμενο.
	int prime_no = sizeof(prime_sizes) / sizeof(int); // το μέγεθος του πίνακα
	for (int i = 0; i < prime_no; i++)
	{ // LCOV_EXCL_LINE
		if (prime_sizes[i] > old_capacity)
		{
			map->capacity = prime_sizes[i];
			break;
		}
	}
	// Αν έχουμε εξαντλήσει όλους τους πρώτους, διπλασιάζουμε
	if (map->capacity == old_capacity) // LCOV_EXCL_LINE
		map->capacity *= 2;			   // LCOV_EXCL_LINE

	// Δημιουργούμε ένα μεγαλύτερο hash table
	map->array = malloc(map->capacity * sizeof(BList));
	for (int i = 0; i < map->capacity; i++)
	{
		map->array[i] = blist_create((DestroyFunc)destroy_map_node);
	}

	// Τοποθετούμε τα παλιά στοιχεία στο νέο hash table
	map->size = 0;
	for (int i = 0; i < old_capacity; i++)
	{

		if (blist_size(old_array[i]) != 0)
		{

			for (BListNode node = blist_first(old_array[i]);
				 node != BLIST_EOF;
				 node = blist_next(old_array[i], node))
			{

				MapNode m_node = blist_node_value(old_array[i], node);
				map_insert(map, m_node->key, m_node->value);
			}
		}
		blist_set_destroy_value(old_array[i], free);
		blist_destroy(old_array[i]);
	}

	//Αποδεσμεύουμε τον παλιό πίνακα ώστε να μήν έχουμε leaks
	free(old_array);
}

// Εισαγωγή στο hash table του ζευγαριού (key, item). Αν το key υπάρχει,
// ανανέωση του με ένα νέο value, και η συνάρτηση επιστρέφει true.

void map_insert(Map map, Pointer key, Pointer value)
{
	
	int pos = map->hash_function(key) % map->capacity;
	struct map_node search_node = {.key = key, .owner = map};

	MapNode node = blist_find(map->array[pos], &search_node, (CompareFunc)compare_map_nodes);
	// Αν υπαρχει ηδη στον πινακα
	if (node != NULL)
	{

		// Κανουμε destroy τις παλιες τιμες και τις αντικαθιστουμε
		if (key != node->key && map->destroy_key != NULL)
			map->destroy_key(node->key);

		if (value != node->value && map->destroy_value != NULL)
			map->destroy_value(node->value);

		node->key = key;
		node->value = value;
	}
	else
	{

		// Αν δεν υπαρχει δεσμευουμε χωρο για το νεο κομβο
		// και τον προσθετουμε
		node = malloc(sizeof(*node));
		node->key = key;
		node->value = value;
		node->array_index = map->hash_function(key) % map->capacity;
		node->owner = map;

		
		blist_insert(map->array[node->array_index], BLIST_EOF, node);
	 	map->size++;
	}
	

	// Αν με την νέα εισαγωγή ξεπερνάμε το μέγιστο load factor, πρέπει να κάνουμε rehash
	float load_factor = (float)map->size / map->capacity;
	if (load_factor > MAX_LOAD_FACTOR)
		rehash(map);

	return;
}

// Διαργραφή απο το Hash Table του κλειδιού με τιμή key
bool map_remove(Map map, Pointer key)
{
	int pos = map->hash_function(key) % map->capacity;
	struct map_node search_node = { .key = key, .owner = map };

	MapNode node = blist_find(map->array[pos], &search_node, (CompareFunc)compare_map_nodes);
	if (node == NULL)
		return false;

	// Αν ο κομβος υπαρχει,βρισκουμε το αντιστοιχο ListNode του 
	// και το αφαιρουμε
	BListNode b_node = blist_find_node(map->array[pos], &search_node, (CompareFunc)compare_map_nodes);
	blist_remove(map->array[pos],b_node );

	return true;
}

// Αναζήτηση στο map, με σκοπό να επιστραφεί το value του κλειδιού που περνάμε σαν όρισμα.
Pointer map_find(Map map, Pointer key)
{

	struct map_node search_node = {.key = key, .owner = map};

	int pos = map->hash_function(key) % map->capacity;

	MapNode node = blist_find(map->array[pos], &search_node, (CompareFunc)compare_map_nodes);
	return node == NULL ? NULL : node->value;
}

DestroyFunc map_set_destroy_key(Map map, DestroyFunc destroy_key)
{

	DestroyFunc old = map->destroy_key;
	map->destroy_key = destroy_key;
	return old;
}

DestroyFunc map_set_destroy_value(Map map, DestroyFunc destroy_value)
{

	DestroyFunc old = map->destroy_value;
	map->destroy_value = destroy_value;
	return old;
}

// Απελευθέρωση μνήμης που δεσμεύει το map
void map_destroy(Map map)
{

	//κανουμε destroy ολες τις λιστες του πινακα,το ιδιο το array και τελος το map
	for (int i = 0; i < map->capacity; i++)
	{
		
		blist_destroy(map->array[i]);
	}

	free(map->array);
	free(map);
	return;
}

/////////////////////// Διάσχιση του map μέσω κόμβων ///////////////////////////

MapNode map_first(Map map)
{

	// Ψαχνουμε για την πρωτη μη κενη λιστα του πινακα
	for (int i = 0; i < map->capacity; i++)
	{

		if (blist_size(map->array[i]) != 0)
		{

			// Αν βρεθει επιστρεφουμε το πρωτο στοιχειο της
			BListNode node = blist_first(map->array[i]);
			return blist_node_value(map->array[i], node);
		}
	}

	return MAP_EOF;
}

MapNode map_next(Map map, MapNode node)
{

	// Βρίσκω το αντίστοιχο BListNode
	BListNode b_node = blist_find_node(map->array[node->array_index], node, (CompareFunc)compare_map_nodes);

	// Βρίσκω το επόμενο BListNode
	BListNode b_node_next = blist_next(map->array[node->array_index], b_node);

	if (b_node_next != NULL)
	{

		// Αν ο κόμβος υπάρχει, επιστέφω το αντίστοιχο MapNode
		return blist_node_value(map->array[node->array_index], b_node_next);
	}
	else
	{

		// Αν δεν υπάρχει , κοιτάω αν υπάρχει κάποια επόμενη μη κενή λίστα
		for (int i = node->array_index + 1; i < map->capacity; i++)
		{

			if (blist_size(map->array[i]) != 0)
			{

				// Αν υπάρχει βρίσκω το πρώτο στοιείο της
				BListNode b_first_node = blist_first(map->array[i]);
				// και επιστρεφω το αντίστοιχο MapNode
				return blist_node_value(map->array[i], b_first_node);
			}
		}
	}

	// Αλλιώς επιστρέφω MAP_EOF
	return MAP_EOF;
}

Pointer map_node_key(Map map, MapNode node)
{

	return node->key;
}

Pointer map_node_value(Map map, MapNode node)
{

	return node->value;
}

MapNode map_find_node(Map map, Pointer key)
{

	int pos = map->hash_function(key) % map->capacity;

	struct map_node search_node = {.key = key, .owner = map};

	return blist_find(map->array[pos], &search_node, (CompareFunc)compare_map_nodes);
}

// Αρχικοποίηση της συνάρτησης κατακερματισμού του συγκεκριμένου map.
void map_set_hash_function(Map map, HashFunc func)
{

	map->hash_function = func;
}

uint hash_string(Pointer value)
{

	// djb2 hash function, απλή, γρήγορη, και σε γενικές γραμμές αποδοτική
	uint hash = 5381;
	for (char *s = value; *s != '\0'; s++)
		hash = (hash << 5) + hash + *s; // hash = (hash * 33) + *s. Το foo << 5 είναι γρηγορότερη εκδοχή του foo * 32.
	return hash;
}

uint hash_int(Pointer value) 
{

	return *(int *)value;
}

uint hash_pointer(Pointer value)
{

	return (size_t)value; // cast σε sizt_t, που έχει το ίδιο μήκος με έναν pointer
}