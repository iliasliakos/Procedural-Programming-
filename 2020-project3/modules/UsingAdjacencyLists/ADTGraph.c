///////////////////////////////////////////////////////////
//
// Υλοποίηση του ADT Graph μέσω λιστών γειτνίασης.
//
///////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "ADTGraph.h"
#include "ADTMap.h"
#include "ADTList.h"
#include "ADTBList.h"
#include "ADTSet.h"
#include "ADTPriorityQueue.h"

struct graph
{

    Map map;
    CompareFunc compare;
    DestroyFunc destroy;
    HashFunc hush_function;
    int size;
};

struct edge
{

    uint weight_edge;        // Το βάρος της ακμής
    Pointer adjacent_vertex; // Η κορυφή με την οποία συνδεέται η κορυφή που μας ενδιαφέρει
    Graph owner;
};

typedef struct edge *Edge;

struct dis_node
{

    uint weight;            // Το βάρος κάθε κορυφής από το σύνολο W ως αυτήν (INT_MAX αν δεν υπάρχει σύνδεση)
    PriorityQueueNode node; // Ο κόμβος στον οποίο βρίσκεται η κορυφή μέσα στην ουρά προτεραιότητας
};

typedef struct dis_node *DisNode;

struct pq_node
{

    Pointer vertex; // Κορυφή του γράφου
    uint weight;    // Το βάρος κάθε κορυφής από το σύνολο W ως αυτήν (INT_MAX αν δεν υπάρχει σύνδεση)
    Graph owner;
};

typedef struct pq_node *PqNode;

// Συνάρτηση που συγκρίνει δύο δείκτες σε struct edge
static int compare_edge_nodes(Edge a, Edge b)
{
    return a->owner->compare(a->adjacent_vertex, b->adjacent_vertex);
}

// Συνάρτηση που συγκρίνει δύο δείκτες σε struct pq_node
static int compare_pq_node(PqNode a, PqNode b)
{

    if (a->weight == b->weight)
    {

        return 0;
    }
    else if (a->weight > b->weight)
    {

        return -1;
    }

    return 1;
}

Graph graph_create(CompareFunc compare, DestroyFunc destroy_vertex)
{

    Graph graph = malloc(sizeof(*graph));
    graph->compare = compare;
    graph->destroy = destroy_vertex;
    graph->size = 0;
    graph->map = map_create(compare, destroy_vertex, destroy_vertex);

    return graph;
}

int graph_size(Graph graph)
{

    return graph->size;
}

void graph_insert_vertex(Graph graph, Pointer vertex)
{

    // Στο map αποθηκεύω κορυφές ως key και δείκτες σε λίστες ως value
    // οι οποιές περιέχουν τους γείτονες και τα βάρη για κάθε κορυφή αντίστοιχα
    map_insert(graph->map, vertex, blist_create(free));

    // Ενημέρωση του size
    graph->size++;
}

List graph_get_vertices(Graph graph)
{

    // Δημιουργία της λιστας
    List list_of_verteces = list_create(NULL);

    // Διάσχιση του map
    for (MapNode node = map_first(graph->map);
         node != MAP_EOF;
         node = map_next(graph->map, node))
    {

        // Βρίσκω το vertex και το προσθέτω στη λιστα
        Pointer value = map_node_key(graph->map, node);
        list_insert_next(list_of_verteces, LIST_BOF, value);
    }

    return list_of_verteces;
}

void graph_remove_vertex(Graph graph, Pointer vertex)
{

    // Βρίσκει τη λίστα με τους γείτονες
    // της vertex και την κάνει destroy
    BList list = map_find(graph->map, vertex);
    blist_destroy(list);

    map_set_destroy_value(graph->map, NULL);

    // Destroy το vertex
    bool i = map_remove(graph->map, vertex);

    map_set_destroy_value(graph->map, free);
    
    // Ενημέρωση του size
    if (i == 1)
    {

        graph->size--;
    }

    return;
}

void graph_insert_edge(Graph graph, Pointer vertex1, Pointer vertex2, uint weight)
{

    // Βρίσκε τη λίστα με τους γείτονες της vertex1
    BList vertex1_list = map_find(graph->map, vertex1);

    // Δέσμευση χώρου
    Edge node = malloc(sizeof(*node));
    node->adjacent_vertex = vertex2;
    node->weight_edge = weight;
    node->owner = graph;

    // Προσθέτω στη λίστα γειτνίασης το node
    blist_insert(vertex1_list, BLIST_EOF, node);
}

void graph_remove_edge(Graph graph, Pointer vertex1, Pointer vertex2)
{

    // Bρίσκω την λίστα γειτνίασης της vertex1
    BList vertex1_list = map_find(graph->map, vertex1);

    // Βρίσκω στη λίστα τον κόμβο με κορυφή vertex2
    // και τον διαγράφω
    struct edge search_node = {.adjacent_vertex = vertex2, .owner = graph};
    BListNode node = blist_find_node(vertex1_list, &search_node, (CompareFunc)compare_edge_nodes);
    blist_remove(vertex1_list, node);

    return;
}

uint graph_get_weight(Graph graph, Pointer vertex1, Pointer vertex2)
{

    // Bρίσκω την λίστα γειτνίασης της vertex1
    BList vertex1_list = map_find(graph->map, vertex1);

    // Βρίσκω στη λίστα τον κόμβο με κορυφή vertex2
    struct edge search_node = {.adjacent_vertex = vertex2, .owner = graph};
    BListNode node = blist_find_node(vertex1_list, &search_node, (CompareFunc)compare_edge_nodes);

    // Αν ο κόμβος υπάρχει επιστέφω το βάρος
    if (node != NULL)
    {

        Edge value = blist_node_value(vertex1_list, node);
        return value->weight_edge;
    }

    return UINT_MAX;
}

List graph_get_adjacent(Graph graph, Pointer vertex)
{

    // Δημιουργία της λίστας που θα περιέχει τους γείτονες της κορυφής vertex
    List adjacent_list = list_create(NULL);

    // Bρίσκω την λίστα γειτνίασης της vertex
    BList vertex_list = map_find(graph->map, vertex);

    // Διάσχιση της λίστας
    for (BListNode node = blist_first(vertex_list);
         node != BLIST_EOF;
         node = blist_next(vertex_list, node))
    {

        // Βρίσκω την γειτονική κορυφή και την προσθέτω
        // στη λίστα που θα επιστρέψω
        Edge value = blist_node_value(vertex_list, node);
        list_insert_next(adjacent_list, LIST_BOF, value->adjacent_vertex);
    }

    return adjacent_list;
}

List graph_shortest_path(Graph graph, Pointer source, Pointer target)
{

    // Δημιουργία της λίστας που θα επιστρέψει η συνάρτηση
    List list_shortest_path = list_create(NULL);

    // Δημιουργία του σύνολου W το οποίο περιέχει τις κορυφές που έχουν επισκεφθεί
    // και αρχικοποίηση του συνόλου με τη κορυφή source
    Set W = set_create(graph->compare, NULL);
    set_insert(W, source);

    // Δημιουργλια του συνόλου dis το οποίο αντιστοιχεί κάθε κορυφή με ένα δείκτη σε struct dis_node
    Map dis = map_create(graph->compare, NULL, free);
    map_set_hash_function(dis, graph->hush_function);

    // Δημιουργία του συνόλου prev που αντιστοιχεί κάθε κορυφή
    // με την  προηγούμενη κορυφη  στο βέλτιστο μονοπάτι
    Map prev = map_create(graph->compare, NULL, NULL);
    map_set_hash_function(prev, graph->hush_function);

    // Δημιουργία της ουρας προτεραιότητας
    // η οποία θα περιέχει δείκτες σε sturct pq_node
    PriorityQueue pqueue = pqueue_create((CompareFunc)compare_pq_node, NULL, NULL);

    // Λίστα με όλες τις κορυφές του γράφου
    List list_of_verteces = graph_get_vertices(graph);

    // Διάσχιση της λίστας
    // και αποθήκευση όλως των κορυφών του γράφου στα σύνολο div και στην pqueue
    for (ListNode node = list_first(list_of_verteces);
         node != LIST_EOF;
         node = list_next(list_of_verteces, node))
    {

        Pointer vertex = list_node_value(list_of_verteces, node);

        // Όλες οι κορυφές εκτός απο τη source
        if (graph->compare(source, vertex) != 0)
        {

            // Δέσμευση μνήμης για το PqNode
            PqNode node_pq = malloc(sizeof(*node_pq));
            node_pq->vertex = vertex;
            // Το βάρος από την κορυφή source προς στην τρέχουσα κορυφή
            // αφού το W ακόμα περιέχει σαν στοιχείο μονο τη κορυφή source
            node_pq->weight = graph_get_weight(graph, source, vertex);
            node_pq->owner = graph;
            
            // Δέσμευση μνήμης για το DisNode
            DisNode node_dis = malloc(sizeof(*node_dis));
            // Εισαγωγή του PqNode στην pqueue και αποθήκευση του κόμβου που προσθέθηκε
            node_dis->node = pqueue_insert(pqueue, node_pq);
            // Το βάρος από την κορυφή source προς στην τρέχουσα κορυφή
            // αφού το W ακόμα περιέχει σαν στοιχείο μονο τη κορυφή source
            node_dis->weight = graph_get_weight(graph, source, vertex);

            // Εισαγωγή της τρέχουσας κορυφής και αντιστοίχιση στο κατάλληλο DisNode
            map_insert(dis, vertex, node_dis);

            // Aν η τρέχουσα κορυφή συνδέεται με τη κορυφή source τότε η source
            // είναι η προηγούμενη της κορυφή στο βέλτιστο μονοπάτι
            if (graph_get_weight(graph, source, vertex) != UINT_MAX)
            {

                // Ενημέρωση του συνόλου prev
                map_insert(prev, vertex, source);
            }
            // Αλλιώς σημαίνει οτι δεν έχει προηγούμενη κορυφή
            else
            {

                map_insert(prev, vertex, NULL);
            }
        }
    }

    // Αποδεσμεύω τη λίστα με τις κορυφές όλου του γράφου
    list_destroy(list_of_verteces);

    // Αν η λίστα αδειάσει σημαίνει ότι η κορυφή source και η κορυφή target δεν συνδέονται
    // και θα επιστραφεί κενή λίστα
    while (pqueue_size(pqueue) != 0)
    {

        // Βρίσκω το στοιχείο που έχει το μικρότερο βάρος από το σύνολο
        PqNode min_node = pqueue_max(pqueue);
       
        // Αν το στοιχείο αυτο  έχει βάρος UINT_MAX από το W
        // σημαίνει ότι δεν υπάρχει βέλτιστο μονοπάτι
        if (min_node->weight == UINT_MAX)
        {

            break;
        }

        // Αφαιρεί το στοιχείο από την ουρά,
        // δεν κάνει destory το PqNode του
        pqueue_remove_max(pqueue);

        // Ενημέρωση του συνόλου W
        set_insert(W, min_node->vertex);

        // Λίστα με τους γείτονες τις κορυφής που μπήκε τελευταία στο σύνολο W
        List neighbors_of_min_vertex = graph_get_adjacent(graph, min_node->vertex);

        // Διάσχιση της λίστας
        for (ListNode node = list_first(neighbors_of_min_vertex);
            node != LIST_EOF;
            node = list_next(neighbors_of_min_vertex, node))
        {

            Pointer neighbor_vertex = list_node_value(neighbors_of_min_vertex, node);
            
            // Αν η τρέχουσα γειτονική κορυφή δεν ανοίκει στο σύνολο W
            if (set_find_node(W, neighbor_vertex) == NULL)
            {

                // Βρίσκω το DisNode της τρέχουσας κορυφής στο σύνολο dis και
                // το χρησιμοποιώ για να βρώ το βάρος της απο το σύνολο W
                DisNode neighbor_vertex_node = map_find(dis, neighbor_vertex);

                // Το βάρος απο την κορυφή που μπήκε στο σύνολο W προς
                // τη τρέχουσα γειτονική της κορυφή
                uint weight_from_min_vertex_to_neighbor_vertex = graph_get_weight(graph, min_node->vertex, neighbor_vertex);
                
                // Αν το βάρος απο το W προς τη κορυφής που μπήκε τελευταία στο W + το βάρος
                // από αυτήν στη γειτονική της κορυφή είναι μικρότερο από το βάρος
                //  της γειτονικής κορυφής από το W
                if (weight_from_min_vertex_to_neighbor_vertex != UINT_MAX)
                {

                    if ((neighbor_vertex_node->weight) > (min_node->weight + weight_from_min_vertex_to_neighbor_vertex))
                    {
                      
                        // Ενημέρωση του συνόλου prev με την κορυφή που μπήκε τελευταία στο W
                        // ως προηγούμενη κορυφή της τρέχουσας γειτονικής της κορυφής
                        map_insert(prev, neighbor_vertex, min_node->vertex);

                        /////////////////////////////////////////////////////////
                        // Ενημέρωση της τρέχουσας γειτονικής κορυφής        ////
                        // στο συνόλο div και στη pqueue με το νέο βάρος της ////
                        // από το σύνολο W                                   ////
                        /////////////////////////////////////////////////////////

                        // Ενημέρωη του βάρους του στοιχείου στην ουρά
                        PqNode update_node = pqueue_node_value(pqueue, neighbor_vertex_node->node);
                        update_node->weight = min_node->weight + weight_from_min_vertex_to_neighbor_vertex;
                        pqueue_update_order(pqueue, neighbor_vertex_node->node);
                        
                        // Ενημέρωση του νέου βάρους στο σύνολο dis
                        neighbor_vertex_node->weight = min_node->weight + weight_from_min_vertex_to_neighbor_vertex;
                    }
                }
            }
        }

        // Destroy τη λίστα με τους γείτονες της κορυφής
        // που μπήκε στο W
        list_destroy(neighbors_of_min_vertex);

        // Αν η κορυφή του είναι η κορυφή target σημαίνει
        // ότι βρήκε το βέλτιστο μονοπάτι
        if (graph->compare(target, min_node->vertex) == 0)
        {

            // Εισαγωγή της κορυφής source στη λίστα
            list_insert_next(list_shortest_path, LIST_BOF, min_node->vertex);

            // Βρίσκει την προηγούμενη κορυφή του target
            // στο βέλτιστο μονοπάτι
            Pointer value = map_find(prev, min_node->vertex);

            // Αν η προηγούμενη κορυφή
            // είναι η κορυφή source σημαίνει ότι το βέλτιστο μονοπάτι
            // αποθηκεύτηκε στη λίστα
            while (graph->compare(value, source) != 0)
            {

                // Όσο είναι διαφορετική εισάγω κάθε φορά στην αρχή της λίστας
                // την προηγούμενη κορυφή
                list_insert_next(list_shortest_path, LIST_BOF, value);

                value = map_find(prev, value);
            }
            // Εισαγωγή της κορυφής source στη  αρχή της λίστας
            list_insert_next(list_shortest_path, LIST_BOF, value);

            // Destroy τα σύνολα dis,prev και την ουρά 
            map_destroy(dis);
            map_destroy(prev);
            set_destroy(W);
            pqueue_set_destroy_value(pqueue, free);
            pqueue_destroy(pqueue);

            // Free το min_node καθώς η pqueue έχει NULL
            // ως DestroyFunc
            free(min_node);

            // Επιστρέφω τη λίστα
            return list_shortest_path;
        }

        // Free το min_node καθώς η pqueue έχει NULL
        // ως DestroyFunc
        free(min_node);
    }

    // Destroy τα σύνολα dis,prev και την ουρά
    map_destroy(dis);
    map_destroy(prev);
    set_destroy(W);
    pqueue_set_destroy_value(pqueue, free);
    pqueue_destroy(pqueue);

    // Αν φτάσει εδώ , επιστροφή κενής λίστας
    return list_shortest_path;
}

void graph_destroy(Graph graph)
{

    // Διάσχιση του map
    for (MapNode node = map_first(graph->map);
         node != MAP_EOF;
         node = map_next(graph->map, node))
    {

        // destroy τη λίστα γειτνίασης
        BList list = map_node_value(graph->map, node);
        blist_destroy(list);
    }

    map_set_destroy_value(graph->map, NULL);
    // Destroy το map
    map_destroy(graph->map);

    free(graph);
}

void graph_set_hash_function(Graph graph, HashFunc hash_func)
{
    graph->hush_function = hash_func;
    map_set_hash_function(graph->map, hash_func);
}
