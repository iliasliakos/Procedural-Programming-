#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ADTBList.h"
#include "ADTMap.h"

// Δομή που περιέχει τις πληροφοριές ενός φοιτητή 
struct student_information{

    int id;
    char* first_name;
    char* last_name;
    int zip_code;
    int year;
    double average;
    BListNode b_node; 
};

typedef struct student_information* StudentInfo;

struct year_node{
	int year;
	BList student_info_list; // λίστα με StudentsInfo
};

typedef struct year_node* YearNode;


struct postal_node{

    int zip_code;    
    int count;
};

typedef struct postal_node* PostalNode;

// compare συνάρτηση που συγκρίνει ακεραίους
int compare_ints(Pointer a, Pointer b) {

	return *(int*)a - *(int*)b;
}

// compare συνάρτηση που συγκρίνει doubles
int compare_doubles(Pointer a, Pointer b){

    if (*(double*)a > *(double*)b) return -1;
    else if (*(double*)a < *(double*)b) return 1;
    return 0;
}

// compare συνάρτηση που συγκρίνει τα YearNodes με βάση το year
int compare_YearNode(Pointer a, Pointer b){

    YearNode a1 = a;
    YearNode b1 = b;

    return compare_ints(&a1->year, &b1->year);
}

// compare συνάρτηση που συγκρίνει τα StudentInfo με βάση το id
int compare_StudentInfo(Pointer a, Pointer b){

    StudentInfo a1 = a;
    StudentInfo b1 = b;

    return compare_ints(&a1->id, &b1->id);
}

// compare συνάρτηση που συγκρίνει τα PostalNode με βάση το zip_code
int compare_PostalNode(Pointer a, Pointer b){

    PostalNode a1 = a;
    PostalNode b1 = b;

    return compare_ints(&a1->zip_code, &b1->zip_code);
}

// Επιστρέφει έναν δείκτη ακέραιο σε νέα μνήμη με τιμή value
int* create_int(int value) {
	int* p = malloc(sizeof(int));
	*p = value;
	return p;
}


// Συνάρτηση που υπολογίζει και επιστρέφει το πλήθος των γραμμών ενός αρχείου
int countlines(char* file_name){

    FILE *fp;
        // counter για τις γραμμές του αρχείου 
        int count = 0;   
        char c;   
  
        // άνοιγμα του αρχείου
        fp = fopen(file_name, "r"); 
  
        // έλεγχος 
        if (fp == NULL) 
        { 
            printf("Could not open file %s", file_name); 
            return 0; 
        } 
  
        // προσπέλαση όλων των χαρακτήρων του αρχείου 
        for (c = getc(fp); c != EOF; c = getc(fp)){

            // αν ο τρέχων χαρακτήρας είναι αυτός της αλλαγής γραμμής 
            if (c == '\n') 
                // αύξηση του counter κατα 1 
                count = count + 1; 
        }

        // κλείσιμο του αρχείου 
        fclose(fp); 

        return count;
} 


int main(int argc, char* argv[]){

    // δημιουργία της λίστα με τα YearNode
    BList list_year = blist_create(NULL);

    // δημιουργία λίστας με τα PostaCode
    BList list_postal_code = blist_create(NULL);

    // υπολογισμός των γραμμών του αρχείου (δηλαδή των εγγραφών)
    int count = countlines(argv[2]);
    printf("%d\n", count);

    // για να μην υπάρχει θέμα με τα collisions πρέπει το 
    // load factor του hashtable να είναι κάτω από το 0.9
    int capacity = (int)(count / 0.5);

    Map map = map_create(compare_ints, NULL, NULL, capacity);
    map_set_hash_function(map, hash_int);

    FILE* fp;
    char first_name[20], last_name[20];
    int zip_code, year;
    double average;
    int id;

    // άνοιγμα του αρχείου
    fp = fopen(argv[2], "r");

    // έλεγχος
    if(fp == NULL){

        printf("Error: fopen() failed\n");
        exit(EXIT_FAILURE);
    }

    
    // Βρόγχος επανάληψης στον οποίο διαβάζονται από το αρχείο 6 τιμές σε κάθε γραμμή του
    while(1){
        //printf("mpike\n");
        if(fscanf(fp, "%d%s%s%d%d%lf", &id, last_name, first_name, &zip_code, &year, &average) != 6)
            break;
        
        // δέσμευση μνήμης για μια μεταβλητή τύπου StudentInfo
        StudentInfo value = malloc(sizeof(*value));
        // ενχώρηση των τιμών στα αντίστοιχα πεδία
        value->id = id;
        value->first_name = strdup(first_name);
        value->last_name = strdup(last_name);
        value->zip_code = zip_code;
        value->year = year;
        value->average = average;
    
        // έλεγχος για το αν υπάρχει ήδη τέτοια εγγραφή
        if(map_find_node(map, &id) == MAP_EOF){
            
            // γίνεται η εγγραφή
            map_insert(map,create_int(id), value);

            // ελέγχος για τον αν υπάρχει λίστα με μαθητές για τη χρονία year
            struct year_node search_node = {.year = year};
            if(blist_find_node(list_year, &search_node, (CompareFunc)compare_YearNode) == BLIST_EOF){
                // αν δεν υπάρχει
                // δεσμεύεται μνήμη για YearNode
                YearNode node = malloc(sizeof(*node));
                node->year = year;
                // δημιουργία της λίστας που περιέχει StudentInfo για κάθε φοιτητή 
                // της συγκεκριμένης χρονιάς
                node->student_info_list = blist_create(NULL);
                // εισαγαγή του StudentInfo του φοιτητή με student_id το id
                blist_insert(node->student_info_list, BLIST_EOF, map_node_value(map, map_find_node(map, &id)));

                // εισαγωγή στη λίστα με τα years
                blist_insert(list_year, BLIST_EOF, node);

                // ενημέρωση του b_node
                value->b_node = blist_last(node->student_info_list);
            }
            else{
                // αν υπάρχει ήδη λίστα για τη χρονία year
                // την βρίσκουμε 
                struct year_node search_node = {.year = year};
                YearNode node = blist_find(list_year, &search_node, (CompareFunc)compare_YearNode);

                // ενημέρωση της λίστας της
                blist_insert(node->student_info_list, BLIST_EOF, map_node_value(map, map_find_node(map, &id)));

                // ενημέρωση του b_node
                value->b_node = blist_last(node->student_info_list);
            }

            // έλεγχος για το αν υπάρχει PostalNode για το ΤΚ zip_code
            struct postal_node search_zip_node = {.zip_code = zip_code};
            if(blist_find_node(list_postal_code, &search_zip_node, (CompareFunc)compare_PostalNode) == BLIST_EOF){
                // αν δεν υπάρχει
                // δεσμεύεται μνήμη για PostalNode
                PostalNode p_node = malloc(sizeof(*p_node));
                p_node->zip_code = zip_code;
                // count = 1 αφού μόλις ένας φοιτητής  έχει εγγραφή με αυτόν το postal code
                p_node->count = 1;

                // εισαγωγή στη λίστα με τα years
                blist_insert(list_postal_code, BLIST_EOF, p_node);   
            }
            else{
                // αν υπάρχει ήδη PostalNode για το ΤΚ zip_code
                // την βρίσκουμε 
                struct postal_node search_zip_node = {.zip_code = zip_code};
                PostalNode p_node = blist_find(list_postal_code, &search_zip_node, (CompareFunc)compare_PostalNode);

                // άυξηση του count κατα 1
                p_node->count++;
                
            }

            // εμφάνιση σχετικού μηνύματος
            printf("student %d inserted\n", id);
        }
        else{
            // αν υπάρχει εμφανίζεται σχετικό μήνυμα λάθους
            // και η εισαγωγή δεν γίνεται
            printf("student %d exists\n", id);

            // free τη μνήμη που δεσμεύτηκε
            free(value->first_name);
            free(value->last_name);
            free(value);
        }
        
    }

    // κλείσιμο αρχείου
    fclose(fp);

    const char s[2] = " ";
    while(1){

        // ζηάει από το χρήστη να δώσει εντολή,
        // την διαβάζει και τη αποθηκευεί σαν αλφαριθμητικό στον πίνακα str
        printf("Enter command\n");
        char str[100];
        fgets(str, 100, stdin);

        // στη συνέχεια, σκοπός είναι να πάρουμε τις λέξεις που βρίσκονται ανάμεσα στα κενά 
        char *word;
        // η πρώτη λέξη
        word = strtok(str, s);

        // αν η πρώτη λέξη είναι το i σημαίνει ότι ο χρήστης
        // θέλει να κάνει insert
        if(strcmp(word, "i") == 0){

            // δεσμεύεται μνήμη για StudentInfo μεταβλητή
            StudentInfo value = malloc(sizeof(*value));
            
            int i = 1;
            int flag = 0;
            while( word != NULL){

                word = strtok(NULL, s);
                if(word != NULL){
                    if(i == 1){
                        // η δεύτερη λέξη είναι το id
                        // το οποίο είναι αποθηκευμένο σε αλφαριθμητικό 
                        // και μετατρέπεται σε ακέραιο με την atoi
                        value->id = atoi(word);
                    }
                    if(i == 2){

                        value->last_name = strdup(word);
                    }
                    if(i == 3){

                        value->first_name = strdup(word);
                    }
                    if(i == 4){

                        value->zip_code = atoi(word);
                    }
                    if(i == 5){

                        value->year = atoi(word);
                    }
                    if(i == 6){
                        
                        value->average = atof(word);
                        break;
                    }
                }
                else{
                    flag = 1;
                    break;
                }

                i++;      
            } 

            // έλεγχος για το αν δόθηκε σωστά η εντολή
            if(flag == 0){
                // έλεγχος για το αν υπάρχει ήδη εγγραφή με αυτό το id
                if(map_find_node(map, &value->id) == MAP_EOF){
                    // αν δεν υπάρχει, γινέται η εισαγωγή της εγγραφής
                    map_insert(map, create_int(value->id), value);

                    // ελέγχος για τον αν υπάρχει λίστα με μαθητές για τη χρονία year
                    struct year_node search_node = {.year = value->year};
                    if(blist_find_node(list_year, &search_node, (CompareFunc)compare_YearNode) == BLIST_EOF){
                        // αν δεν υπάρχει
                        // δεσμεύεται μνήμη για YearNode
                        YearNode node = malloc(sizeof(*node));
                        node->year = value->year;
                        // δημιουργία της λίστας που περιέχει StudentInfo για κάθε φοιτητή 
                        // της συγκεκριμένης χρονιάς
                        node->student_info_list = blist_create(NULL);
                        // εισαγαγή του StudentInfo του φοιτητή με student_id το id
                        blist_insert(node->student_info_list, BLIST_EOF, map_node_value(map, map_find_node(map, &value->id)));

                        // εισαγωγή στη λίστα με τα years
                        blist_insert(list_year, BLIST_EOF, node);

                        // ενημέρωση του b_node
                        value->b_node = blist_last(node->student_info_list);
                    }
                    else{
                        // αν υπάρχει ήδη λίστα για τη χρονία year
                        // την βρίσκουμε 
                        struct year_node search_node = {.year = value->year};
                        YearNode node = blist_find(list_year, &search_node, (CompareFunc)compare_YearNode);

                        // ενημέρωση της λίστας της
                        blist_insert(node->student_info_list, BLIST_EOF, map_node_value(map, map_find_node(map, &value->id)));

                        // ενημέρωση του b_node
                        value->b_node = blist_last(node->student_info_list);
                    }

                    // έλεγχος για το αν υπάρχει λίστα για το ΤΚ zip_code
                    struct postal_node search_zip_node = {.zip_code = zip_code};
                    if(blist_find_node(list_postal_code, &search_zip_node, (CompareFunc)compare_PostalNode) == BLIST_EOF){
                        // αν δεν υπάρχει
                        // δεσμεύεται μνήμη για PostalNode
                        PostalNode p_node = malloc(sizeof(*p_node));
                        p_node->zip_code = zip_code;
                        // count = 1 αφού μόλις ένας φοιτητής  έχει εγγραφή με αυτόν το postal code
                        p_node->count = 1;

                        // εισαγωγή στη λίστα με τα years
                        blist_insert(list_postal_code, BLIST_EOF, p_node);   
                    }
                    else{
                        // αν υπάρχει ήδη λίστα για το ΤΚ zip_code
                        // την βρίσκουμε 
                        struct postal_node search_zip_node = {.zip_code = zip_code};
                        PostalNode p_node = blist_find(list_postal_code, &search_zip_node, (CompareFunc)compare_PostalNode);

                        // άυξηση του count κατα 1
                        p_node->count++;
                        
                    }

                    // εμφάνιση σχετικού μηνύματος
                    printf("student %d inserted\n", value->id);
                }
                else{
                    // αν υπάρχει εμφανίζεται σχετικό μήνυμα λάθους
                    // και η εισαγωγή δεν γίνεται
                    printf("student %d exists\n", value->id);

                    // free τη μνήμη που δεσμεύτηκε
                    free(value->first_name);
                    free(value->last_name);
                    free(value);
                }
            }
            else{
                // δόθηκα λιγότερα ορίσματα
                printf("expected 6 args\n");
            }
        }
        // αν η πρώτη λέξη είναι l
        else if (strcmp(word, "l") == 0){
            int id;

            while( word != NULL){

                // βρίσκω τη δεύτερη λέξη που είναι ένα id
                word = strtok(NULL, s);
                // μετατροπή σε ακέραιο
                id = atoi(word);

                break;
            }

            // έλεγχος αν υπάρχει το id
            MapNode node_id = map_find_node(map, &id);
            // Αν δεν υπάρχει
            if(node_id == MAP_EOF){
                // εμφάνιση σχετικού μυνήματος
                printf("Student %d does not exist\n", id);
            }
            //  αν υπάρχει
            else{
                // εύρεση του value του id
                StudentInfo value = map_node_value(map, node_id);

                printf("%d %s %s %d %d %lf\n", id, value->last_name, value->first_name, value->zip_code, value->year, value->average);
            }
            
        }
        // αν η πρώτη λέξη είναι d
        else if(strcmp(word, "d") == 0){
            int id;

            while( word != NULL){

                // βρίσκω τη δεύτερη λέξη που είναι ένα id
                word = strtok(NULL, s);
                // μετατροπή σε ακέραιο
                id = atoi(word);

                break;
            }

            // έλεγχος αν υπάρχει το id
            if(map_find_node(map, &id) == MAP_EOF){
                // αν δεν υπάρχει εμφάνιση σχετικού μηνύματος
                printf("Student %d does not exist\n", id);
            }
            else{
                StudentInfo s_node = map_node_value(map, map_find_node(map, &id));

                // βρίσκω τη λίστα του ΤΚ s_node->zip_code
                struct postal_node search_p_node = {.zip_code = s_node->zip_code};
                PostalNode p_node = blist_find(list_postal_code, &search_p_node, (CompareFunc)compare_PostalNode);

                // // μείωση του p_node->count κατά 1
                // p_node->count--;

                // βρίσκω τη λίστα της χρονίας s_node->year
                struct year_node search_node = {.year = s_node->year};
                YearNode node = blist_find(list_year, &search_node, (CompareFunc)compare_YearNode);

                // free το s_node
                // αφαίρεση από την λίστα της το σχετικό StudentInfo
                free(s_node->first_name);
                free(s_node->last_name);
                blist_remove(node->student_info_list, s_node->b_node);
                free(s_node);

                // αφαίρεση απο το map
                map_set_destroy_key(map, free);
                map_remove(map, &id);

                // εμφάνιση σχετικού μηνύματος
                printf("Record %d deleted\n", id);
            }
        }
        else if(strcmp(word, "n") == 0){
            int year;

            while( word != NULL){

                // βρίσκω τη δεύτερη λέξη που είναι ένα year
                word = strtok(NULL, s);
                // μετατροπή σε ακέραιο
                year = atoi(word);

                break;
            }

            // βρίσκω τη λίστα με χρονία year 
            struct year_node search_node = {.year = year};
            YearNode node = blist_find(list_year, &search_node, (CompareFunc)compare_YearNode);

            if( node != NULL){
                // αν υπάρχουν εκτυπώνω το σχετικό μήνυμα
                printf("%d students in %d\n", blist_size(node->student_info_list), year);
            }
            else{
                printf(" No students enrolled in %d\n", year);
            }
        }
        else if(strcmp(word, "t") == 0){
            int y = 2;
            int num;
            int year;

            while( word != NULL){

                word = strtok(NULL, s);
                // βρίσκω τη δεύτερη λέξη που είναι ένα num
                if(y == 2){
                    // μετατροπή σε ακέραιο
                    num = atoi(word);
                }
                // βρίσκω τη 3 λέξη που είναι ένα year
                if(y == 3){
                    // μετατροπή σε ακέραιο
                    year = atoi(word);
                    break;
                }

                y++;
            }

            // βρίσκω τη λίστα με χρονία year 
            struct year_node search_node = {.year = year};
            YearNode node = blist_find(list_year, &search_node, (CompareFunc)compare_YearNode);
            printf("%d\n", blist_size(node->student_info_list));

            if(node != NULL){
                // counter για το array
                int i = 0;

                // πίνακας με τα gpa
                double gpa_array[blist_size(node->student_info_list)];

                // προσπέλαση της λίστας
                for(BListNode b_node = blist_first(node->student_info_list);
                    b_node != BLIST_EOF;
                    b_node = blist_next(node->student_info_list, b_node)){

                        StudentInfo value = blist_node_value(node->student_info_list, b_node);

                        
                        // εκχώρηση του average στο πίνακα
                        gpa_array[i] = value->average;
                        i++;
                    }

                // ταξινόμηση του πίνακα σε φθίνουσα σειρά
                qsort(gpa_array, blist_size(node->student_info_list), sizeof(double), (CompareFunc)compare_doubles);

                // εκτύπωση των id που το average τους ανοίκει στις num πρώτες θέσεις του πίνακα
                // προσπέλαση της λίστας
                for(BListNode b_node = blist_first(node->student_info_list);
                    b_node != BLIST_EOF;
                    b_node = blist_next(node->student_info_list, b_node)){

                        StudentInfo value = blist_node_value(node->student_info_list, b_node);

                        for(int i = 0;i <= num-1; i++){
                            //printf("%f\n",gpa_array[i]);
                            if(fabs(value->average - gpa_array[i]) < 0.0000001){
                                printf("%d ", value->id);
                                break;
                            }
                        }
                    }
                
                printf("\n");
            }
            else{
                printf("No students enrolled in %d\n", year);
            }
        }
        else if(strcmp(word, "a") == 0){
            int year;

            while( word != NULL){

                // βρίσκω τη δεύτερη λέξη που είναι ένα year
                word = strtok(NULL, s);
                // μετατροπή σε ακέραιο
                year = atoi(word);

                break;
            }

            // βρίσκω τη λίστα με χρονία year 
            struct year_node search_node = {.year = year};
            YearNode node = blist_find(list_year, &search_node, (CompareFunc)compare_YearNode);

            double sum = 0;
            // προσπέλαση της λίστας και αποθήκευση των average σε έναν sum
            for(BListNode b_node = blist_first(node->student_info_list);
                b_node != BLIST_EOF;
                b_node = blist_next(node->student_info_list, b_node)){

                    StudentInfo value = blist_node_value(node->student_info_list, b_node);
                    sum = sum + value->average;
                }
            
            // αν η χρονιά year είχε φοιτητές
            if(blist_size(node->student_info_list) != 0){
                // εκτύπωση του μέσου όρου τους
                printf("%f\n", sum/blist_size(node->student_info_list));
            }
            else{
                printf("No students enrolled in %d\n", year);
            }
        }
        else if(strcmp(word, "m") == 0){

            int year;

            while( word != NULL){

                // βρίσκω τη δεύτερη λέξη που είναι ένα year
                word = strtok(NULL, s);
                // μετατροπή σε ακέραιο
                year = atoi(word);

                break;
            }

            // βρίσκω τη λίστα με χρονία year 
            struct year_node search_node = {.year = year};
            YearNode node = blist_find(list_year, &search_node, (CompareFunc)compare_YearNode);

            if(node != NULL && blist_size(node->student_info_list) != 0){
                
                int i = 0;
                double average_array[blist_size(node->student_info_list)];
                // προσπέλαση της λίστας και αποθήκευση των average σε έναν πίνακα
                for(BListNode b_node = blist_first(node->student_info_list);
                    b_node != BLIST_EOF;
                    b_node = blist_next(node->student_info_list, b_node)){

                        StudentInfo value = blist_node_value(node->student_info_list, b_node);
                        average_array[i] = value->average;
                        i++;
                    }
                
                // ταξινόμηση του πίνακα σε φθίνουσα σειρά  
                qsort(average_array, blist_size(node->student_info_list), sizeof(double), compare_doubles);

                // προσπέλαση της λίστας 
                // και εκτύπωση των id που o μέσος όρος τους ισσούται
                // με το τελευταίο στοιχείο του πίνακα
                for(BListNode b_node = blist_first(node->student_info_list);
                    b_node != BLIST_EOF;
                    b_node = blist_next(node->student_info_list, b_node)){

                        StudentInfo value = blist_node_value(node->student_info_list, b_node);
                        
                        if(fabs(value->average - average_array[blist_size(node->student_info_list)-1]) < 0.0000001){
                            // εκτύπωση του id
                            printf("%d ", value->id);
                        }  
                    }
                    printf("\n");   
            }
            else{
                printf("No students enrolled in %d\n", year);
            }    

        }
        else if(strcmp(word, "c\n") == 0){

            if(blist_size(list_year) != 0){
                // προσπέλαση της λίστας year και εκτύπωση του μεγέθους της λίστας κάθε χρονιάς
                for(BListNode b_node = blist_first(list_year);
                    b_node != BLIST_EOF;
                    b_node = blist_next(list_year, b_node)){

                        YearNode value = blist_node_value(list_year, b_node);

                        printf("{%d , %d} ",value->year, blist_size(value->student_info_list));  
                    }
                    printf("\n");
            }
            else{
                printf("No students enrolled\n");
            }   
        }
        else if(strcmp(word, "p") == 0){
            int rank;

            while( word != NULL){

                // βρίσκω τη δεύτερη λέξη που είναι ένα year
                word = strtok(NULL, s);
                // μετατροπή σε ακέραιο
                rank = atoi(word);

                break;
            }

            int size = blist_size(list_postal_code);
            int postal_array[size];
            // προσπέλαση της λίστα
            // και εκχώρηση των count στο postal_array
            int y = 0;
            for(BListNode node = blist_first(list_postal_code);
                node != BLIST_EOF;
                node = blist_next(list_postal_code, node)){

                   PostalNode p_node = blist_node_value(list_postal_code, node);

                   postal_array[y] = p_node->count;
                   y++; 
                }

                // ταξινόμηση του postal_array σε αύξουσα σειρά
                qsort(postal_array, size, sizeof(int), compare_ints);

                // προσπέλαση της λίστας και εμφάνιση 
                // όσων zip_codes  τα counts είναι ίσα με το size-rank στοιχείο του πίνακα
                for(BListNode node = blist_first(list_postal_code);
                node != BLIST_EOF;
                node = blist_next(list_postal_code, node)){

                   PostalNode p_node = blist_node_value(list_postal_code, node);

                   if(p_node->count == postal_array[size-rank]){
                       printf("%d is %d most popular\n",p_node->zip_code, rank);
                   } 
                }

        }
        else if(strcmp(word, "e\n") == 0){
            break;
        }
    }

    // free τη μνήμη της inverted index
    for(BListNode node = blist_first(list_year);
        node != BLIST_EOF;
        node = blist_next(list_year, node)){

            YearNode year_node = blist_node_value(list_year, node);
            for(BListNode s_node = blist_first(year_node->student_info_list);
                s_node != BLIST_EOF;
                s_node = blist_next(year_node->student_info_list, s_node)){

                    StudentInfo value = blist_node_value(year_node->student_info_list, s_node);
                    free(value->first_name);
                    free(value->last_name);
                    free(value);
                }
            blist_destroy(year_node->student_info_list);
            free(year_node);
        }
    blist_destroy(list_year);

    // free τη μνήμη της list_postal_code
    for(BListNode p_node = blist_first(list_postal_code);
        p_node != BLIST_EOF;
        p_node = blist_next(list_postal_code, p_node)){

        PostalNode value = blist_node_value(list_postal_code, p_node);
        free(value);
        }
    blist_destroy(list_postal_code);    

    // free τη μνήμη του map
    map_set_destroy_key(map, free);
    map_destroy(map);
}