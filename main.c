/* C program for simulating FAT-16
 *
 * ZID - Z1835018
 * Course - CSCI 580
 *
 * Author - Akhil Pandey (https://akhilpandey95.com, https://github.com/akhilpandey95)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>

/*
 * KM - the numerical value of a kilobyte
 * MB - the numerical value of a megabyte
 * HOW_OFTEN - the number of times we must print the value stored in the two lists
 *
 */
#define TEMP 100
#define HOW_OFTEN 2
#define NUM_BYTES 512
#define MAX_BLK_ENTRIES 12

/* NOTE: setbuf() doesnot redirect the output to a buffer, instead it
 * would tell the standard I/O library to use that particular buffer.
 * So keeping these technicalities in mind a better way to flush out
 * the buffer would be fflush().
 */

/*
* struct text_file [T_File] - is a struct that has information about the input text file
* struct mem_block [M_Block] - is a struct that has information about the memory block
* struct entry_object [Entry_Obj] - is a struct that has information about the file stored in the diretory
*
*/
typedef char Content;

typedef struct text_file {
    size_t size;
    size_t capacity;
    Content *array;
} T_File;

typedef struct mem_block {
    int starting_addr;
    int size_of_block;
    char **proc_id_owner;
    char **id_of_block;
} M_Block;

typedef struct entry_object {
    char **file_name;
    char **size;
    char **file_blocks;
    int first_block_index;
    int last_block_index;
    int *n;
    int **n1;
    int block_count;
    int file_count;
    struct Node* blocks;
} Entry_Obj;

typedef struct Node {
    int data;
    struct Node* next;
} Node;

typedef void (*callback)(Node* data);

/*
* Items part of a Node Struct
* -----------------------
* count()             - Counts the number of elements in the linked list
* create()            - Creates a empty linked list
* display()           - Displays elements of the linked list
* remove_element()    - Removes elements from the linked list
* -----------------------
*
*/

Node* get_node(int data);
void push(Node** head_ref, int new_data);
void insert_after(Node* prev_node, int new_data);
void insert_at_n(Node* head_ref, int x, int n);
void append(Node** head_ref, int new_data);
char* display_list(int* n, int size);
void display_fat(Node* n);
void traverse_the_list(Node *n, callback f);

/*
* -----------------------------
 // * Definitions of Node struct
 * -------------------------------
 */

Node* get_node(int data) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->data = data;
    newNode->next = NULL;
}

void push(Node** head_ref, int new_data) {
    struct Node* new_node = (Node*) malloc(sizeof(Node));
    new_node->data  = new_data;
    new_node->next = (*head_ref);
    (*head_ref) = new_node;
}

void insert_after(Node* prev_node, int new_data) {
    if (prev_node == NULL) {
        fprintf(stderr, "the given previous node cannot be NULL\n");
        fflush(stderr);
        return;
    }
    Node* new_node =(Node*) malloc(sizeof(Node));
    new_node->data  = new_data;
    new_node->next = prev_node->next;
    prev_node->next = new_node;
}

void insert_at_n(Node* head, int n, int x) {
    // if list is empty
    if (head == NULL)
        return;

    // get a new node for the value 'x'
    Node* newNode = get_node(x);
    Node* ptr = head;
    int len = 0, i;

    // find length of the list, i.e, the
    // number of nodes in the list
    while (ptr != NULL) {
        len++;
        ptr = ptr->next;
    }

    // traverse up to the nth node from the end
    ptr = head;
    for (i = 1; i <= (len - n); i++)
        ptr = ptr->next;

    // insert the 'newNode' by making the
    // necessary adjustment in the links
    newNode->next = ptr->next;
    ptr->next = newNode;
}

void append(Node** head_ref, int new_data) {
    Node* new_node = (Node*) malloc(sizeof(Node));
    Node *last = *head_ref;
    new_node->data  = new_data;
    new_node->next = NULL;

    if (*head_ref == NULL) {
        *head_ref = new_node;
        return;
    }

    while (last->next != NULL) {
        last = last->next;
    }
    last->next = new_node;
    return;
}

void delete_node(Node **head_ref, int position) {
   // If linked list is empty
   if (*head_ref == NULL)
      return;

   // Store head node
    Node* temp = *head_ref;

    // If head needs to be removed
    if (position == 0)
    {
        *head_ref = temp->next;   // Change head
        free(temp);               // free old head
        return;
    }

    // Find previous node of the node to be deleted
    for (int i=0; temp!=NULL && i<position-1; i++)
         temp = temp->next;

    // If position is more than number of ndoes
    if (temp == NULL || temp->next == NULL)
         return;

    // Node temp->next is the node to be deleted
    // Store pointer to the next of node to be deleted
    Node *next = temp->next->next;

    // Unlink the node from linked list
    free(temp->next);  // Free memory

    temp->next = next;  // Unlink the deleted node from list
}

char* display_list(int* n, int size) {
    int i = 0;
    char s[1024];
    char *array = malloc(1 * sizeof(char*));
    for (i = 0; i < size; i++) {
        snprintf(s, sizeof(s), "%d", n[i]);
        strcat(array, s);
        strcat(array, ",");
    }
    return array;
}

void display_fat(Node* n) {
    int k = 1;
    fprintf(stdout, "File Allocation Table: \n-----------------------------------\n");
    fflush(stdout);
    while (n != NULL) {
        if (k % MAX_BLK_ENTRIES == 0) {
            fprintf(stdout, "%d\t\n", n->data);
            fflush(stdout);
        } else {
            fprintf(stdout, "%d\t", n->data);
            fflush(stdout);
        }
        n = n->next;
        k++;
    }
    fprintf(stdout, "-----------------------------------\n");
    fflush(stdout);
}

void traverse_the_list(Node *n, callback f) {
    Node* cursor = n;
    while (cursor != NULL) {
        f(cursor);
        cursor = cursor->next;
    }
}

/*
* Items part of a Text File Struct
* -----------------------
* t_file_size()            - Understand the size of the given text file
* t_file_input()           - Process the text file by taking its file pointer as input
* t_file_ret_arr()         - Return the elements of the text file in an array
* t_file_fill_content()    - Fill the array with apropriate information present in text file
* t_file_create_vector()   - create an appropriate array for storing information of text file
* t_file_set_zero_size()   - return the size of the given array, NOTE: this depends on text file
* t_file_destroy_vector()  - destroy the information present in the created array
* -----------------------
*
*/
T_File *t_file_create_vector();
void t_file_fill_content(T_File *tf, Content value);
void t_file_set_zero_size(T_File *tf);
size_t t_file_size(T_File *tf);
Content *t_file_ret_arr(T_File *tf);
void t_file_destroy_vector(T_File *tf);
char *t_file_input(FILE *fp);

/*
* -----------------------------
 // * Definitions of T_File struct
 * -------------------------------
 */
T_File *t_file_create_vector() {
    T_File *tf;
    tf = (T_File*)malloc(sizeof(T_File));
    if (tf) {
        tf->size = 0;
        tf->capacity = 16;
        tf->array = (Content*)realloc(NULL, sizeof(Content)*(tf->capacity += 16));
    }
    return tf;
}

void t_file_fill_content(T_File *tf, Content value) {
    tf->array[tf->size] = value;
    if (++tf->size == tf->capacity) {
        tf->array = (Content*)realloc(tf->array, sizeof(Content)*(tf->capacity += 16));
        if (!tf->array) {
            perror("memory not enough");
            exit(-1);
        }
    }
}

void t_file_set_zero_size(T_File *tf) {
    tf->size = 0;
}

size_t t_file_size(T_File *tf) {
    return tf->size;
}

Content *t_file_ret_arr(T_File *tf) {
    return tf->array;
}

void t_file_destroy_vector(T_File *tf) {
    free(tf->array);
    free(tf);
}

char *t_file_input(FILE *fp) {
    static T_File *tf = NULL;
    int ch;

    if (tf == NULL) {
        tf = t_file_create_vector();
    }
    t_file_set_zero_size(tf);
    while (EOF != (ch=fgetc(fp))) {
        if (isspace(ch)) {
            continue;
        }
        while (!isspace(ch)) {
            t_file_fill_content(tf, ch);
            if(EOF == (ch = fgetc(fp))) {
                break;
            }
        }
        t_file_fill_content(tf, '\0');
        break;
    }
    if (t_file_size(tf) != 0) {
        return t_file_ret_arr(tf);
    }
    t_file_destroy_vector(tf);
    tf = NULL;
    return NULL;
}

// declaration of the function init_process_sync() which would
// abbreviate to start the process synchronization
int init_fat12_simulation();

/* declaration of sub functions that are necessary for simulating
 * the memory management task
 *
 * best_fit_mem_allocation()  - Implementation of best fit memory allocation algorithm
 * first_fit_mem_allocation() - Implementation of first fit memory allocation algorithm
 *
 */
Entry_Obj fat_copy_transaction(Node* head, Entry_Obj files_in_dir, char **proc_information, int index, int k);
Entry_Obj fat_delete_transaction(Node* head, Entry_Obj files_in_dir, char **proc_information, int index, int k);
Entry_Obj fat_new_file_transaction(Node* head, Entry_Obj files_in_dir, char **proc_information, int index, int k);
Entry_Obj fat_modify_file_transaction(Node* head, Entry_Obj files_in_dir, char **proc_information, int index, int k);
Entry_Obj fat_rename_file_transaction(Entry_Obj files_in_dir, char **proc_information, int index, int k);
char** remove_element(char **array, int size_of_array, int index_to_remove);
int parse_info_from_file(FILE *fp, Node* head);
int print_files_in_dir(Entry_Obj *files_in_dir);
int return_ascii(char **data, int index);
int* give_free_blocks(Entry_Obj* files_in_dir, Node* head, int how_many);
int does_file_exist(char **proc_information, Entry_Obj files_in_dir, int index, int k);

/* defining the function does_file_exist() in order
 * to call it in the init_memory_sim() function.
 *
 * @param {void*} thread_id - The thread id of the created thread
 */
int does_file_exist(char **proc_information, Entry_Obj files_in_dir, int index, int k) {
    int i = 0;
    int check = -1;

    for (i = 0; i < files_in_dir.file_count; i++) {
        if ((strcmp(proc_information[index+1], files_in_dir.file_name[i])) == 0) {
            check = 1;
        }
    }
    return check;
}

/* defining the function fat_copy_transaction() in order
 * to call it in the init_memory_sim() function.
 *
 * @param {void*} thread_id - The thread id of the created thread
 */
Entry_Obj fat_copy_transaction(Node* head, Entry_Obj files_in_dir, char **proc_information, int index, int k) {
    int i = 0;
    char **size = malloc(1 * sizeof(char*));

    // check if the file exists in the directory or not
    int s_ind = does_file_exist(proc_information, files_in_dir, index, k);
    int c_ind = does_file_exist(proc_information, files_in_dir, index+1, k);

    if (s_ind == 1 && c_ind == -1) {
            // clear the existing file in order to adjust the chronological order
            while (i < k - 1) {
                if (!(strcmp(proc_information[index+1], files_in_dir.file_name[i]))) {
                    size[0] = files_in_dir.size[i];
                    // printf("fc: %d\n", files_in_dir.file_count);
                    break;
                }
                i++;
            }

            // file name
            files_in_dir.file_name = (char**)realloc(files_in_dir.file_name, (files_in_dir.file_count+1)*sizeof(*files_in_dir.file_name));
            files_in_dir.file_name[files_in_dir.file_count] = (char*)malloc(sizeof(proc_information[index+2]));
            strcpy(files_in_dir.file_name[files_in_dir.file_count], proc_information[index+2]);

            // file size
            files_in_dir.size = (char**)realloc(files_in_dir.size, (files_in_dir.file_count+1)*sizeof(*files_in_dir.size));
            files_in_dir.size[files_in_dir.file_count] = (char*)malloc(sizeof(proc_information[index+2]));
            strcpy(files_in_dir.size[files_in_dir.file_count], size[0]);

            // calculate how many blocks are required
            files_in_dir.block_count = (atoi(size[0]) / 512) + 1;

            // file blocks
            files_in_dir.n = give_free_blocks(&files_in_dir, head, files_in_dir.block_count);

            // // file blocks
            files_in_dir.file_blocks = (char**)realloc(files_in_dir.file_blocks, (files_in_dir.file_count+1)*sizeof(*files_in_dir.file_blocks));
            files_in_dir.file_blocks[files_in_dir.file_count] = (char*)malloc((files_in_dir.block_count + 100) * sizeof(display_list(files_in_dir.n, files_in_dir.block_count)));
            strcpy(files_in_dir.file_blocks[files_in_dir.file_count], display_list(files_in_dir.n, files_in_dir.block_count));
            // strcpy(files_in_dir.file_blocks[files_in_dir.file_count], "None");

            // printf("%d, %d: %s \n", files_in_dir.n[files_in_dir.block_count], files_in_dir.file_count, display_list(files_in_dir.n, files_in_dir.block_count));

            // insert the blocks to the fat table
            if (files_in_dir.block_count == 1) {
                insert_at_n(head, files_in_dir.n[files_in_dir.block_count] - i, -1);
                delete_node(&head, i + (240 - (files_in_dir.n[files_in_dir.block_count])));
            } else {
                for (i = 0; i < files_in_dir.block_count; i++) {
                    if (i == files_in_dir.block_count - 1) {
                        insert_at_n(head, files_in_dir.n[files_in_dir.block_count] - i, -1);
                        delete_node(&head, i + (240 - (files_in_dir.n[files_in_dir.block_count])));
                    } else {
                        insert_at_n(head, files_in_dir.n[files_in_dir.block_count] - i, files_in_dir.n[i+1]);
                        delete_node(&head, i + (240 - (files_in_dir.n[files_in_dir.block_count])));
                    }
                    // printf("insert: %d \n", files_in_dir.n[files_in_dir.block_count] - i, files_in_dir.n[i]);
                    // printf("delete: %d \n", i + (240 - (files_in_dir.n[files_in_dir.block_count])));
                }
            }

            // file count
            files_in_dir.file_count += 1;

    }
    if (c_ind == 1) {
        fprintf(stderr, "ERROR: The output file %s already exists. Cannot Copy\n", proc_information[index+2]);
        fflush(stderr);
    }
    if (s_ind == -1) {
        fprintf(stderr, "ERROR: The input file %s doesn't exist in the directory. Cannot Copy\n", proc_information[index+1]);
        fflush(stderr);
    }
    return files_in_dir;
}

/* defining the function fat_delete_transaction() in order
 * to call it in the init_memory_sim() function.
 *
 * @param {void*} thread_id - The thread id of the created thread
 */
Entry_Obj fat_delete_transaction(Node* head, Entry_Obj files_in_dir, char **proc_information, int index, int k) {
    int i = 0;
    int count = 0;
    int prev_size = 0;
    int prev_index = 0;

    // tokenizing the previous blocks and storing them
    char *token;
    char *n;

    // check if the file exists in the directory or not
    int s_ind = does_file_exist(proc_information, files_in_dir, index, k);

    if (s_ind == 1) {
        // clear the existing file in order to adjust the chronological order
        while (i < k - 1) {
            if (!(strcmp(proc_information[index+1], files_in_dir.file_name[i]))) {
                prev_size = atoi(files_in_dir.size[i]);
                prev_index = i;
                token = strtok(files_in_dir.file_blocks[i], ",");
                files_in_dir.file_name = remove_element(files_in_dir.file_name, k, i);
                files_in_dir.size = remove_element(files_in_dir.size, k, i);
                files_in_dir.file_blocks = remove_element(files_in_dir.file_blocks, k, i);
                files_in_dir.file_count -= 1;
                break;
            }
            i++;
        }

        n = malloc(((prev_size/512) + 1) * sizeof(int));

        while (token != NULL && count < ((prev_size/512) + 1)) {
            n[count] = atoi(token);
            token = strtok(NULL, ",");
            count++;
        }

        // first delete the old blocks
        for (i = 0; i < ((prev_size/512) + 1); i++) {
            insert_at_n(head, 240 - n[i], 0);
            delete_node(&head, n[i]);
        }
    } else {
        fprintf(stderr, "ERROR: The file %s doesn't exist in the directory. Cannot Delete\n", proc_information[index+1]);
        fflush(stderr);
    }

    return files_in_dir;
}

/* defining the function fat_new_file_transaction() in order
 * to call it in the init_memory_sim() function.
 *
 * @param {void*} thread_id - The thread id of the created thread
 */
Entry_Obj fat_new_file_transaction(Node* head, Entry_Obj files_in_dir, char **proc_information, int index, int k) {
    int i = 0;
    int blocks_required = 0;

    // check if the file exists in the directory or not
    int s_ind = does_file_exist(proc_information, files_in_dir, index, k);

    if (s_ind == -1) {
        // check if the file size is 0
        if (atoi(proc_information[index+2]) == 0) {
            // file name
            files_in_dir.file_name = (char**)realloc(files_in_dir.file_name, (files_in_dir.file_count+1)*sizeof(*files_in_dir.file_name));
            files_in_dir.file_name[files_in_dir.file_count] = (char*)malloc(sizeof(proc_information[index+1]));
            strcpy(files_in_dir.file_name[files_in_dir.file_count], proc_information[index+1]);

            // file size
            files_in_dir.size = (char**)realloc(files_in_dir.size, (files_in_dir.file_count+1)*sizeof(*files_in_dir.size));
            files_in_dir.size[files_in_dir.file_count] = (char*)malloc(sizeof(proc_information[index+2]));
            strcpy(files_in_dir.size[files_in_dir.file_count], proc_information[index+2]);

            // // file blocks
            files_in_dir.file_blocks = (char**)realloc(files_in_dir.file_blocks, (files_in_dir.file_count+1)*sizeof(*files_in_dir.file_blocks));
            files_in_dir.file_blocks[files_in_dir.file_count] = (char*)malloc((files_in_dir.block_count + 100) * sizeof(display_list(files_in_dir.n, files_in_dir.block_count)));
            strcpy(files_in_dir.file_blocks[files_in_dir.file_count], "NONE");

            // file count
            files_in_dir.file_count += 1;
        } else {
            // file name
            files_in_dir.file_name = (char**)realloc(files_in_dir.file_name, (files_in_dir.file_count+1)*sizeof(*files_in_dir.file_name));
            files_in_dir.file_name[files_in_dir.file_count] = (char*)malloc(sizeof(proc_information[index+1]));
            strcpy(files_in_dir.file_name[files_in_dir.file_count], proc_information[index+1]);

            // file size
            files_in_dir.size = (char**)realloc(files_in_dir.size, (files_in_dir.file_count+1)*sizeof(*files_in_dir.size));
            files_in_dir.size[files_in_dir.file_count] = (char*)malloc(sizeof(proc_information[index+2]));
            strcpy(files_in_dir.size[files_in_dir.file_count], proc_information[index+2]);

            // calculate how many blocks are required
            files_in_dir.block_count = (atoi(proc_information[index+2]) / 512) + 1;

            // file blocks
            files_in_dir.n = give_free_blocks(&files_in_dir, head, files_in_dir.block_count);

            // // file blocks
            files_in_dir.file_blocks = (char**)realloc(files_in_dir.file_blocks, (files_in_dir.file_count+1)*sizeof(*files_in_dir.file_blocks));
            files_in_dir.file_blocks[files_in_dir.file_count] = (char*)malloc((files_in_dir.block_count + 100) * sizeof(display_list(files_in_dir.n, files_in_dir.block_count)));
            strcpy(files_in_dir.file_blocks[files_in_dir.file_count], display_list(files_in_dir.n, files_in_dir.block_count));
            // strcpy(files_in_dir.file_blocks[files_in_dir.file_count], "None");

            // printf("%d, %d: %s \n", files_in_dir.n[files_in_dir.block_count], files_in_dir.file_count, display_list(files_in_dir.n, files_in_dir.block_count));

            // insert the blocks to the fat table
            if (files_in_dir.block_count == 1) {
                insert_at_n(head, files_in_dir.n[files_in_dir.block_count] - i, -1);
                delete_node(&head, i + (240 - (files_in_dir.n[files_in_dir.block_count])));
            } else {
                for (i = 0; i < files_in_dir.block_count; i++) {
                    if (i == files_in_dir.block_count - 1) {
                        insert_at_n(head, files_in_dir.n[files_in_dir.block_count] - i, -1);
                        delete_node(&head, i + (240 - (files_in_dir.n[files_in_dir.block_count])));
                    } else {
                        insert_at_n(head, files_in_dir.n[files_in_dir.block_count] - i, files_in_dir.n[i+1]);
                        delete_node(&head, i + (240 - (files_in_dir.n[files_in_dir.block_count])));
                    }
                    // printf("insert: %d \n", files_in_dir.n[files_in_dir.block_count] - i, files_in_dir.n[i]);
                    // printf("delete: %d \n", i + (240 - (files_in_dir.n[files_in_dir.block_count])));
                }
            }

            // file count
            files_in_dir.file_count += 1;
        }
    } else {
        fprintf(stderr, "ERROR: The file %s already exists in the directory. Cannot Create.\n", proc_information[index+1]);
        fflush(stderr);
    }
    return files_in_dir;
}

/* defining the function fat_modify_file_transaction() in order
 * to call it in the init_memory_sim() function.
 *
 * @param {void*} thread_id - The thread id of the created thread
 */
Entry_Obj fat_modify_file_transaction(Node* head, Entry_Obj files_in_dir, char **proc_information, int index, int k) {
    int i = 0;
    int j = 0;
    int a = 0;
    int b = 0;
    int prev_size = 0;
    int prev_index = 0;
    int count = 0;

    int *n;
    char *token;

    // check if the file exists in the directory or not
    int s_ind = does_file_exist(proc_information, files_in_dir, index, k);

    if (s_ind == 1) {
            // clear the existing file in order to adjust the chronological order
            while (i < k - 1) {
                if (!(strcmp(proc_information[index+1], files_in_dir.file_name[i]))) {
                    prev_size = atoi(files_in_dir.size[i]);
                    prev_index = i;
                    token = strtok(files_in_dir.file_blocks[i], ",");
                    files_in_dir.file_name = remove_element(files_in_dir.file_name, k, i);
                    files_in_dir.size = remove_element(files_in_dir.size, k, i);
                    files_in_dir.file_blocks = remove_element(files_in_dir.file_blocks, k, i);
                    files_in_dir.file_count -= 1;
                    // printf("fc: %d\n", files_in_dir.file_count);
                    break;
                }
                i++;
            }

            n = malloc(((prev_size/512) + 1) * sizeof(int));

            while (token != NULL && count < ((prev_size/512) + 1)) {
                n[count] = atoi(token);
                token = strtok(NULL, ",");
                count++;
            }

            if (prev_size != 0) {
                // first delete the old blocks
                for (i = 0; i < ((prev_size/512) + 1); i++) {
                    insert_at_n(head, 240 - n[i], 0);
                    delete_node(&head, n[i]);
                }
            }

            // file name
            files_in_dir.file_name = (char**)realloc(files_in_dir.file_name, (files_in_dir.file_count+1)*sizeof(*files_in_dir.file_name));
            files_in_dir.file_name[files_in_dir.file_count] = (char*)malloc(sizeof(proc_information[index+1]));
            strcpy(files_in_dir.file_name[files_in_dir.file_count], proc_information[index+1]);

            // file size
            files_in_dir.size = (char**)realloc(files_in_dir.size, (files_in_dir.file_count+1)*sizeof(*files_in_dir.size));
            files_in_dir.size[files_in_dir.file_count] = (char*)malloc(sizeof(proc_information[index+2]));
            strcpy(files_in_dir.size[files_in_dir.file_count], proc_information[index+2]);

            // calculate how many blocks are required
            files_in_dir.block_count = (atoi(proc_information[index+2]) / 512) + 1;

            // file blocks
            files_in_dir.n = give_free_blocks(&files_in_dir, head, files_in_dir.block_count);

            if (atoi(proc_information[index + 2]) != 0) {
                // file blocks
                files_in_dir.file_blocks = (char**)realloc(files_in_dir.file_blocks, (files_in_dir.file_count+1)*sizeof(*files_in_dir.file_blocks));
                files_in_dir.file_blocks[files_in_dir.file_count] = (char*)malloc((files_in_dir.block_count + 100) * sizeof(display_list(files_in_dir.n, files_in_dir.block_count)));
                strcpy(files_in_dir.file_blocks[files_in_dir.file_count], display_list(files_in_dir.n, files_in_dir.block_count));

                // printf("block count:%d ", files_in_dir.block_count);
                // now add new blocks
                if (files_in_dir.block_count == 1) {
                    insert_at_n(head, files_in_dir.n[files_in_dir.block_count] - i, -1);
                    delete_node(&head, i + (240 - (files_in_dir.n[files_in_dir.block_count])));
                } else {
                    for (i = 0; i < files_in_dir.block_count; i++) {
                        if (i == files_in_dir.block_count - 1) {
                            insert_at_n(head, files_in_dir.n[files_in_dir.block_count] - i, -1);
                            delete_node(&head, i + (240 - (files_in_dir.n[files_in_dir.block_count])));
                        } else {
                            insert_at_n(head,  240 - files_in_dir.n[i], files_in_dir.n[i+1]);
                            delete_node(&head, files_in_dir.n[i]);
                        }
                        // printf("insert: %d \n", files_in_dir.n[files_in_dir.block_count] - i, files_in_dir.n[i]);
                        // printf("delete: %d \n", i + (240 - (files_in_dir.n[files_in_dir.block_count])));
                    }
                }
            } else {
                // file blocks
                files_in_dir.file_blocks = (char**)realloc(files_in_dir.file_blocks, (files_in_dir.file_count+1)*sizeof(*files_in_dir.file_blocks));
                files_in_dir.file_blocks[files_in_dir.file_count] = (char*)malloc((files_in_dir.block_count + 100) * sizeof(display_list(files_in_dir.n, files_in_dir.block_count)));
                strcpy(files_in_dir.file_blocks[files_in_dir.file_count], "NONE");
            }

            files_in_dir.file_count += 1;
    }
    else {
        fprintf(stderr, "\nERROR: The file %s doesn't exist in the directory.Cannot Modify\n", proc_information[index + 1]);
        fflush(stderr);
    }
    return files_in_dir;
}

/* defining the function fat_rename_file_transaction() in order
 * to call it in the init_memory_sim() function.
 *
 * @param {void*} thread_id - The thread id of the created thread
 */
Entry_Obj fat_rename_file_transaction(Entry_Obj files_in_dir, char **proc_information, int index, int k) {
    int i = 0;
    int name_index = 0;

    // check if the file exists in the directory or not
    int s_ind = does_file_exist(proc_information, files_in_dir, index, k);
    int c_ind = does_file_exist(proc_information, files_in_dir, index+1, k);

    if (s_ind == 1 && c_ind == -1) {
        // clear the existing file in order to adjust the chronological order
        while (i < k - 1) {
            if (!(strcmp(proc_information[index+1], files_in_dir.file_name[i]))) {
                name_index = i;
                // printf("fc: %d\n", files_in_dir.file_count);
                break;
            }
            i++;
        }

        // file name
        strcpy(files_in_dir.file_name[name_index], proc_information[index+2]);

    }
    if (c_ind == 1) {
        fprintf(stderr, "\nERROR: The output file %s already exists. Cannot Rename\n", proc_information[index + 2]);
        fflush(stderr);
    }
    if (s_ind == -1) {
        fprintf(stderr, "\nERROR: The input file %s doesn't exist in the directory. Cannot Rename\n", proc_information[index + 1]);
        fflush(stderr);
    }
    return files_in_dir;
}

/* defining the function remove_element() in order
 * to call it in the init_memory_sim() function.
 *
 * @param {void*} thread_id - The thread id of the created thread
 */
char** remove_element(char **array, int size_of_array, int index_to_remove) {
    int i = 0;
    int count = 0;
    char** temp = malloc((size_of_array - 1) * sizeof(char*));

    for (i = 0; i < size_of_array; i++) {
        if (index_to_remove != i) {
            temp[count] = array[i];
            count++;
        }
    }
    return temp;
}

/* defining the function parse_info_from_file() in order
 * to call it in the init_memory_sim() function.
 *
 * @param {void*} thread_id - The thread id of the created thread
 */
int parse_info_from_file(FILE *fp, Node* head) {
    int i = 0;
    int j = 0;
    int k = 0;
    int count = 0;
    int transaction_char;
    int transaction_count = 0;

    // init the entry object
    Entry_Obj files_in_dir;
    files_in_dir.file_name = NULL;
    files_in_dir.size = NULL;
    files_in_dir.file_blocks = NULL;
    files_in_dir.file_count = 0;
    files_in_dir.block_count = 0;
    files_in_dir.blocks = NULL;

    // init the variables for parsing the info from file
    char *words_in_file;
    char **proc_information = NULL;
    while ((words_in_file = t_file_input(fp)) != NULL) {
        if (!strcmp(words_in_file, "?")) {
            break;
        } else {
            i++;
            proc_information = (char**)realloc(proc_information, (i+1)*sizeof(*proc_information));
            proc_information[i-1] = (char*)malloc(sizeof(words_in_file));
            strcpy(proc_information[i-1], words_in_file);
            count++;
        }
    }

    // add "." and ".." to the entry object
    if (files_in_dir.file_count == 0) {
        // insert the file names
        files_in_dir.file_name = (char**)realloc(files_in_dir.file_name, (files_in_dir.file_count+1)*sizeof(*files_in_dir.file_name));
        files_in_dir.file_name[files_in_dir.file_count] = (char*)malloc(sizeof("."));
        strcpy(files_in_dir.file_name[files_in_dir.file_count], ".");

        // update the file sizes
        files_in_dir.size = (char**)realloc(files_in_dir.size, (files_in_dir.file_count+1)*sizeof(*files_in_dir.size));
        files_in_dir.size[files_in_dir.file_count] = (char*)malloc(sizeof("512"));
        strcpy(files_in_dir.size[files_in_dir.file_count], "512");

        // update the file sizes
        files_in_dir.file_blocks = (char**)realloc(files_in_dir.file_blocks, (files_in_dir.file_count+1)*sizeof(*files_in_dir.file_blocks));
        files_in_dir.file_blocks[files_in_dir.file_count] = (char*)malloc(sizeof("0"));
        strcpy(files_in_dir.file_blocks[files_in_dir.file_count], "0");

        //update the file_count
        files_in_dir.file_count = 1;
    }

    // start the parsing and perform the operations accordingly
    for (j = 0; j < TEMP; j++) {
        if ((strlen(proc_information[j]) == 1 && !strcmp(proc_information[j], "N")) || (strlen(proc_information[j]) == 1 && !strcmp(proc_information[j], "C")) || (strlen(proc_information[j]) == 1 && !strcmp(proc_information[j], "D")) || (strlen(proc_information[j]) == 1 && !strcmp(proc_information[j], "M")) || (strlen(proc_information[j]) == 1 && !strcmp(proc_information[j], "R"))) {
            k++;
            transaction_char = return_ascii(proc_information, j);
            switch (transaction_char) {
                case 78:
                    printf("N: %d -> %d, %s\n", k, j, proc_information[j+1]);
                    files_in_dir = fat_new_file_transaction(head, files_in_dir, proc_information, j, k);
                    fflush(stdout);
                    if (transaction_count % HOW_OFTEN == 0) {
                        print_files_in_dir(&files_in_dir);
                        display_fat(head);
                    }
                    transaction_count++;
                    break;
                case 77:
                    printf("M: %d -> %d, %s\n", k, j, proc_information[j+1]);
                    files_in_dir = fat_modify_file_transaction(head, files_in_dir, proc_information, j, k);
                    fflush(stdout);
                    if (transaction_count % HOW_OFTEN == 0) {
                        print_files_in_dir(&files_in_dir);
                        display_fat(head);
                    }
                    transaction_count++;
                    break;
                case 67:
                    printf("C: %d -> %d, %s\n", k, j, proc_information[j+1]);
                    files_in_dir = fat_copy_transaction(head, files_in_dir, proc_information, j, k);
                    fflush(stdout);
                    if (transaction_count % HOW_OFTEN == 0) {
                        print_files_in_dir(&files_in_dir);
                        display_fat(head);
                    }
                    transaction_count++;
                    break;
                case 82:
                    printf("R: %d -> %d, %s\n", k, j, proc_information[j+1]);
                    files_in_dir = fat_rename_file_transaction(files_in_dir, proc_information, j, k);
                    fflush(stdout);
                    if (transaction_count % HOW_OFTEN == 0) {
                        print_files_in_dir(&files_in_dir);
                        display_fat(head);
                    }
                    transaction_count++;
                    break;
                case 68:
                    printf("D: %d -> %d, %s\n", k, j, proc_information[j+1]);
                    files_in_dir = fat_delete_transaction(head, files_in_dir, proc_information, j, k);
                    fflush(stdout);
                    if (transaction_count % HOW_OFTEN == 0) {
                        print_files_in_dir(&files_in_dir);
                        display_fat(head);
                    }
                    transaction_count++;
                    break;
            }
        }
    }
    return 0;
}

/* defining the function print_files_in_dir() in order
 * to call it in the init_fat12_simulation() function.
 *
 * @param {void*} thread_id - The thread id of the created thread
 */
int print_files_in_dir(Entry_Obj *files_in_dir) {
    int i = 0;
    int sum = 0;
    char *s;
    fprintf(stdout, "\nFiles in dir\n----------------------------------------------\n");
    fprintf(stdout, "Name\t\tSize\t\tBlocks\n");
    fflush(stdout);
    for (i = 0; i < files_in_dir->file_count; i++) {
        sum += atoi(files_in_dir->size[i]);
        if (i == 0) {
            fprintf(stdout, "%s\t\t%d\t\t%s\n", files_in_dir->file_name[i], atoi(files_in_dir->size[i]), files_in_dir->file_blocks[i]);
            fflush(stdout);
        }
        else if (i == 1) {
            fprintf(stdout, "%s\t\t%d\t\t%s\n", files_in_dir->file_name[i], atoi(files_in_dir->size[i]), files_in_dir->file_blocks[i]);
            fflush(stdout);
        } else {
            if (strlen(files_in_dir->file_name[i]) < 5) {
                fprintf(stdout, "%s\t\t%d\t\t%s\n", files_in_dir->file_name[i], atoi(files_in_dir->size[i]), files_in_dir->file_blocks[i]);
                fflush(stdout);
            } else {
                fprintf(stdout, "%s\t%d\t\t%s\n", files_in_dir->file_name[i], atoi(files_in_dir->size[i]), files_in_dir->file_blocks[i]);
                fflush(stdout);
            }
        }
    }
    fprintf(stdout, "----------------------------------------------\n");
    fprintf(stdout, "Root Directory Info: %d files, Total Size: %d Bytes\n\n", files_in_dir->file_count, sum);
    fflush(stdout);
    return 0;
}

/* defining the function return_ascii() in order
 * to call it in the init_fat12_simulation() function.
 *
 * @param {void*} thread_id - The thread id of the created thread
 */
int return_ascii(char **proc_information, int j) {
    int transaction_char;
    if (!strcmp(proc_information[j], "N")) {
        transaction_char = 78;
    } else if (!strcmp(proc_information[j], "M")) {
        transaction_char = 77;
    } else if (!strcmp(proc_information[j], "C")) {
        transaction_char = 67;
    } else if (!strcmp(proc_information[j], "R")) {
        transaction_char = 82;
    } else if (!strcmp(proc_information[j], "D")) {
        transaction_char = 68;
    } else {
        transaction_char = 0;
    }
    return transaction_char;
}

/* defining the function give_free_blocks() in order
 * to call it in the init_fat12_simulation() function.
 *
 * @param {void*} thread_id - The thread id of the created thread
 */
int* give_free_blocks(Entry_Obj* files_in_dir, Node* head, int n) {
    int i = 0;
    int j = 0;
    int k = 0;
    int rows = n;
    int cols = 1;
    int *data;
    int count = 0;
    int fcount = 0;
    int value = 0;
    int first_free_block = 0;
    int* blocks = malloc((n + 1) * sizeof(int));

    // entry object related information
    files_in_dir->blocks = NULL;
    data = malloc(rows * cols * sizeof(*data));
    files_in_dir->n1 = malloc(rows * sizeof(files_in_dir->n1));

    while (head != NULL && count < n) {
        if (head->data == 0) {
            append(&files_in_dir->blocks, i);
            value = 240 - i;
            blocks[count] = files_in_dir->blocks->data;
            // printf("%d: %d, ", value, files_in_dir->blocks->data);
            // printf("\n");
            files_in_dir->blocks = files_in_dir->blocks->next;
            count++;
        }
        i++;
        head = head->next;
    }
    blocks[n] = value + n - 1;
    for (j = 0; j < n; j++) {
        data[j] = blocks[j];
    }
    for (k = 0; k < rows; k++) {
        files_in_dir->n1[k] = &data[k * cols];
    }
    return blocks;
}

/* defining the function halt_process_sync() in order to call
 * it in the init_process_sync() function.
 */
int init_fat12_simulation() {
    int i = 0;
    int j = 0;
    int k = 0;
    printf("Beginning the Simulation Run now.\n\n");
    FILE *fp = fopen("data.txt", "r");
    Node *head = NULL;
    Node *temp = NULL;
    for (k =  0; k < 240; k++) {
        if (k == 0) {
            append(&head, -1);
        } else {
            append(&head, 0);
        }
    }
    parse_info_from_file(fp, head);
    printf("\nEnding the Simulation Run\n");
    fflush(stdout);
    return 0;
}

int main() {
    // call the fat12_simulation() function here
    init_fat12_simulation();
    return 0;
}
