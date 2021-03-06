#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DiseaseMonitor.h"
#include "ADTVector.h"
#include "ADTSet.h"
#include "ADTPriorityQueue.h"
#include "ADTMap.h"

// Map που αντιστοιχεί record->disease σε Record
static Map monitor_records = NULL;
// Map που αντιστοιχεί record->id σε Record
static Map id_map = NULL;
// Map που αντιστοιχεί DiseaseCountryDate σε έναν counter
static Map dcd = NULL;

struct disease_country_date
{

    Pointer disease;
    Pointer country;
    Pointer date;
};

typedef struct disease_country_date *DiseaseCountryDate;

struct top_diseases {

    Pointer disease;
    int counter;
};

typedef struct top_diseases* TopDiseases;

int compare_ints(Pointer a, Pointer b)
{

    return *(int *)a - *(int *)b;
}

int compare_strings(Pointer a, Pointer b)
{

    int k = strcmp(a, b);
    return k;
}

int compare_records(Pointer a, Pointer b)
{

    Record a1 = a;
    Record b1 = b;

    return a1->id - b1->id;
}

int compare_dates(Pointer a, Pointer b)
{

    int year_a = atoi(a + 0);
    int year_b = atoi(b + 0);

    if (year_a == year_b)
    {

        int month_a = atoi(a + 5);
        int month_b = atoi(b + 5);

        if (month_a == month_b)
        {

            int day_a = atoi(a + 8);
            int day_b = atoi(b + 8);

            return day_a - day_b;
        }

        return month_a - month_b;
    }

    return year_a - year_b;
}

int compare_DiseaseCountryDate(Pointer a, Pointer b)
{

    DiseaseCountryDate a1 = a;
    DiseaseCountryDate b1 = b;

    if (compare_strings(a1->country, b1->country) == 0 && compare_strings(a1->disease, b1->disease) == 0 && compare_dates(a1->date, b1->date) == 0)
    {
        return 0;
    }
    return 1;
}

// Επιστρέφει έναν ακέραιο σε νέα μνήμη με τιμή value
int *create_int(int value)
{

    int *p = malloc(sizeof(int));
    *p = value;

    return p;
}

void dm_init()
{

    if (monitor_records != NULL)
    {

        dm_destroy();
    }
    // Αν monitor_records == NULL
    else
    {
        ///////////////////////////////////
        // Αρχικοποιήση των δομών
        /////////////////////////////////////////////

        monitor_records = map_create((CompareFunc)compare_strings, NULL, NULL);
        map_set_hash_function(monitor_records, hash_string);

        id_map = map_create((CompareFunc)compare_ints, free, NULL);
        map_set_hash_function(id_map, hash_int);

        dcd = map_create((CompareFunc)compare_DiseaseCountryDate, free, free);
        map_set_hash_function(dcd, hash_pointer);
    }
}

void dm_destroy()
{
    // Destroy το id_map
    map_destroy(id_map);

    // Destroy το monitor_records
    for (MapNode node = map_first(monitor_records);
        node != MAP_EOF;
        node = map_next(monitor_records, node))
    {

        // Destroy το value
        Set value_set = map_node_value(monitor_records, node);
        set_destroy(value_set);
    }
    map_destroy(monitor_records);

    // Destroy το dcd
    map_destroy(dcd);

   

    monitor_records = NULL;
    id_map = NULL;
    dcd = NULL;
}

bool dm_insert_record(Record record)
{

    // Ενημέρωση του id_map
    map_insert(id_map, create_int(record->id), record);

    MapNode node = map_find_node(monitor_records, record->disease);

    bool i = 0;
    // Αν υπάρχει το record->disease στο map
    if (node != NULL)
    {

        // Βρίσκει το αντίστοiχο Set
        Set set = map_find(monitor_records, record->disease);

        // Ελέγχει αν το id υπάρχει ήδη στο Set
        SetNode id_node = set_find_node(set, record);
        if (id_node != NULL)
        {
            i = 1;
        }

        // Προσθέτει στο Set το record
        set_insert(set, record);
    }
    else
    {

        // Αν δεν υπάρχει το record->disease στο map
        // προσθέτει το record->disease ,ως key, στο map,
        // δημιουργεί ένα Set και το αντιστοιχίζει σε αυτό
        map_insert(monitor_records, record->disease, set_create((CompareFunc)compare_records, NULL));

        // Προσθέτει το record στο Set
        Set set_disease = map_find(monitor_records, record->disease);
        set_insert(set_disease, record);
    }

    // Ενημέρωση του Map dcd
    struct disease_country_date search_node = {.disease = record->disease, .country = record->country, .date = record->date};
    int *counter_dcd = map_find(dcd, &search_node);

    // Αν υπάρχει ήδη στο map
    if (counter_dcd != NULL)
    {
        // Αυξάνει το πλήθος των εγγραφών κατά 1
        *counter_dcd = *counter_dcd + 1;
    }
    // Αν δεν υπάρχει
    else
    {
        // Δέσμευση χώρου για το σύνολο {disease, country, record}
        DiseaseCountryDate dcd_node = malloc(sizeof(*dcd_node));
        dcd_node->country = record->country;
        dcd_node->date = record->date;
        dcd_node->disease = record->disease;

        // Προσθήκη στο map με πλήθος εγγραφών 1
        map_insert(dcd, dcd_node, create_int(1));
    }

    return i;
}

bool dm_remove_record(int id)
{

    // Βρίσκει το Record του id
    Record record = map_find(id_map, &id);

    // Το record αποκλίεται να έχει σαν τιμή NULL
    // άρα αν η map_find επιστρέψει NULL σημαίνει
    // ότι δεν υπάρχει το id σαν key
    if (record == NULL)
    {

        return 0;
    }

    // Βρίσκει το Set που αντισοιχεί στο disease του Record
    // και αφαιρεί από το Set το Record
    Set set = map_find(monitor_records, record->disease);
    set_remove(set, record);

    // Ενημέρωση του dcd_map
    struct disease_country_date search_node = {.disease = record->disease, .country = record->country, .date = record->date};
    int *counter_dcd = map_find(dcd, &search_node);

    if (counter_dcd != NULL)
    {
        *counter_dcd = *counter_dcd - 1;
    }

    // Ενημέρωση του id_map
    map_remove(id_map, &id);

    return 1;
}

List dm_get_records(String disease, String country, Date date_from, Date date_to)
{

    // Δημιουργία της λίστας που θα επιστραφεί
    List list = list_create(NULL);

    if (disease != NULL)
    {

        // Βρίσκει το Set με τις εγγραφές τις συγκεκριμένης ασθένειας
        Set set = map_find(monitor_records, disease);

        // Διάσχιση του Set
        for (SetNode node_set = set_first(set);
             node_set != SET_EOF;
             node_set = set_next(set, node_set))
        {

            Record record = set_node_value(set, node_set);

            // Μεταβλητή που παίρνει την τιμή 1
            // όταν οι εγγραφή ικανοποιεί τα κριτήρια
            int flag = 1;
            if (country != NULL)
            {

                if (compare_strings(country, record->country) != 0)
                {

                    flag = 0;
                }
            }

            if (date_from != NULL)
            {

                if (compare_dates(date_from, record->date) > 0)
                {

                    flag = 0;
                }
            }

            if (date_to != NULL)
            {

                if (compare_dates(record->date, date_to) > 0)
                {

                    flag = 0;
                }
            }

            if (flag == 1)
            {

                // Προσθήκη της εγγραφής στη λίστα
                list_insert_next(list, LIST_BOF, record);
            }
        }
    }
    else
    {
        // Αν το disease == NULL
        // δίασχιση του monitor_records
        for (MapNode node = map_first(monitor_records);
             node != MAP_EOF;
             node = map_next(monitor_records, node))
        {

            Set set_value = map_node_value(monitor_records, node);

            for (SetNode node_set1 = set_first(set_value);
                 node_set1 != SET_EOF;
                 node_set1 = set_next(set_value, node_set1))
            {

                Record record = set_node_value(set_value, node_set1);
                int flag = 1;
                if (country != NULL)
                {

                    if (compare_strings(country, record->country) != 0)
                    {

                        flag = 0;
                    }
                }

                if (date_from != NULL)
                {

                    if (compare_dates(date_from, record->date) > 0)
                    {

                        flag = 0;
                    }
                }

                if (date_to != NULL)
                {

                    if (compare_dates(record->date, date_to) > 0)
                    {

                        flag = 0;
                    }
                }

                if (flag == 1)
                {

                    list_insert_next(list, LIST_BOF, record);
                }
            }
        }
    }

    return list;
}

int dm_count_records(String disease, String country, Date date_from, Date date_to)
{
    // Το πλήθος των εγγραφών που ικανοποιούν τα κριτήρια
    int sum = 0;

    // Διάσχιση του dcd
    for (MapNode node = map_first(dcd);
         node != MAP_EOF;
         node = map_next(dcd, node))
    {

        DiseaseCountryDate node_dcd = map_node_key(dcd, node);

        // Μεταβλητή η οποία έχει τιμή 1 όταν
        // το σύνολο {disease, country, date}
        // ικανοποιεί τα κριτήρια
        int flag = 1;
        if (disease != NULL)
        {

            if (compare_strings(disease, node_dcd->disease) != 0)
            {

                flag = 0;
            }
        }

        if (country != NULL)
        {

            if (compare_strings(country, node_dcd->country) != 0)
            {

                flag = 0;
            }
        }

        if (date_from != NULL)
        {

            if (compare_dates(date_from, node_dcd->date) > 0)
            {

                flag = 0;
            }
        }

        if (date_to != NULL)
        {

            if (compare_dates(node_dcd->date, date_to) > 0)
            {

                flag = 0;
            }
        }

        if (flag == 1)
        {
            // Πρόσθεση του πλήθους των εγγραφών
            // στο τελικό άθροισμα
            int *counter_dcd = map_node_value(dcd, node);
            sum = sum + *counter_dcd;
        }
    }

    return sum;
}

