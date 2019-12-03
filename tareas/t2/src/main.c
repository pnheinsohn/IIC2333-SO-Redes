#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <wait.h>
#include <errno.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define MAXWORDS 1000  /* Per proccess */
#define MAXCHILDS 8  /* 9 with main */
#define MAXWORDLEN 46

const char *first_key_path = "./keys/first_key";
const char *second_key_path = "./keys/second_key";
const char *third_key_path = "./keys/third_key";
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;


typedef struct stringSturct {
    int count;
    char *string;
} StringStruct;

typedef struct stringList {
    int length;
    StringStruct **chunk;
} StringList;

typedef struct mapArgs {
    int id;
    int type;
    int max_words;
    int len_words_chunk;
    StringStruct **words_chunk;
} MapArgs;

typedef struct reduceArgs {
    int type;
    int len_words_chunk_list;
    StringList **words_chunk_list;
} ReduceArgs;

MapArgs *init_MapArgs(int type, int max_words, int id, StringStruct **words_chunk) {
    MapArgs *new = malloc(sizeof(MapArgs));
    new -> id = id;
    new -> type = type;
    new -> len_words_chunk = 0;
    new -> max_words = max_words;
    new -> words_chunk = words_chunk;
    return new;
}

ReduceArgs *init_ReduceArgs(int type, int len_words_chunk_list, StringList **words_chunk_list) {
    ReduceArgs *new = malloc(sizeof(ReduceArgs));
    new -> type = type;
    new -> words_chunk_list = words_chunk_list;
    new -> len_words_chunk_list = len_words_chunk_list;
    return new;
}

StringStruct *init_string(char *string) {
    StringStruct* new = malloc(sizeof(StringStruct));
    new -> string = malloc(sizeof(char) * (strlen(string) + 1));
    strcpy(new -> string, string);
    new -> count = 1;
    return new;
}

StringList *init_string_list(int size) {
    StringList *new = malloc(sizeof(StringList));
    new -> chunk = malloc(sizeof(StringStruct*) * size);
    new -> length = 0;
    return new;
}

char *lowerString(char* string) {
    for (int i=0; string[i]; i++) {
        string[i] = tolower(string[i]);
    }
    return string;
}

void bubbleSort(StringStruct **words_chunk, int len_words_chunk) {
    for (int i=0; i < len_words_chunk; i++) { /* Bubble sort */
        for (int j=0; j < len_words_chunk - i - 1; j++) {
            if ((words_chunk[j] -> count > words_chunk[j + 1] -> count) || 
                    (words_chunk[j] -> count == words_chunk[j + 1] -> count && strcmp(words_chunk[j] -> string, words_chunk[j + 1] -> string) > 0)) {
                StringStruct *aux = words_chunk[j];
                words_chunk[j] = words_chunk[j + 1];
                words_chunk[j + 1] = aux;
            }
        }
    }
}

void *addSharedMemory(void **shared_memories, int *shared_index, long *identifiers, int words_amount, int type) {
    key_t key;
    if (type == 1) {
        key = ftok(first_key_path, *shared_index);
    } else if (type == 2) {
        key = ftok(second_key_path, *shared_index);
    } else {
        key = ftok(third_key_path, *shared_index);
    }
    long identifier = shmget(key, sizeof(int) + sizeof(int) * words_amount + sizeof(char) * MAXWORDLEN * words_amount + sizeof(int) * words_amount, IPC_CREAT | SHM_W | SHM_R);
    identifiers[*shared_index] = identifier;
    void *shared_memory = shmat(identifier, NULL, 0);
    shared_memories[*shared_index] = shared_memory;
    (*shared_index)++;
    return shared_memory;
}

void shmWrite(void *shared_memory, StringStruct **words_chunk, int len_words_chunk) {
    void *shmaux = shared_memory;
    int *shmlen = (int *) shmaux;
    *shmlen = len_words_chunk;
    shmaux = shmaux + sizeof(int);
    for (int i=0; i < len_words_chunk; i++) {
        int *shmsize = (int *) shmaux;
        *shmsize = strlen(words_chunk[i] -> string);
        char *shmstring = (char *) (shmaux + sizeof(int));
        strcpy(shmstring, words_chunk[i] -> string);
        int *shmcount = (int *) (shmaux + sizeof(int) + sizeof(char) * (*shmsize + 1));
        *shmcount = words_chunk[i] -> count;
        shmaux = (shmaux + sizeof(int) + sizeof(char) * (*shmsize + 1) + sizeof(int));
    }
    for (int i=0; i < len_words_chunk; i++) {
        free(words_chunk[i] -> string);
        free(words_chunk[i]);
    }
    free(words_chunk);
}

void shmStruct(void *shared_memory, StringStruct **words_chunk, int max_words, int type) {
    int local_len_words_chunk;
    StringStruct** local_words_chunk = malloc(sizeof(StringStruct*) * max_words);
    if (type == 1) {
        for (int i=0; i < max_words; i++) {
            if (i == 0) {
                local_words_chunk[i] = init_string(words_chunk[i] -> string);
                local_len_words_chunk = 1;
            } else {
                for (int j=0; j < local_len_words_chunk; j++) {
                    if (strcmp(local_words_chunk[j] -> string, words_chunk[i] -> string) == 0) {
                        local_words_chunk[j] -> count++;
                        break;
                    } else if (j == local_len_words_chunk - 1) {
                        local_words_chunk[j + 1] = init_string(words_chunk[i] -> string);
                        local_len_words_chunk++;
                        break;
                    }
                }
            }
            free(words_chunk[i] -> string);
            free(words_chunk[i]);
        }
    } else if (type == 2) {
        for (int i=0; i < max_words; i++) {
            if (i == 0) {
                local_words_chunk[i] = init_string(words_chunk[i] -> string);
                local_words_chunk[i] -> count = words_chunk[i] -> count;
                local_len_words_chunk = 1;
            } else {
                for (int j=0; j < local_len_words_chunk; j++) {
                    if (local_words_chunk[j] -> count == words_chunk[i] -> count) {
                        int actual_string_size = strlen(local_words_chunk[j] -> string);
                        local_words_chunk[j] -> string = (char *) realloc(local_words_chunk[j] -> string, (actual_string_size + strlen(words_chunk[i] -> string) + 2));
                        strcat(local_words_chunk[j] -> string, ";");
                        strcat(local_words_chunk[j] -> string, words_chunk[i] -> string);
                        break;
                    } else if (j == local_len_words_chunk - 1) {
                        local_words_chunk[j + 1] = init_string(words_chunk[i] -> string);
                        local_words_chunk[j + 1] -> count = words_chunk[i] -> count;
                        local_len_words_chunk++;
                        break;
                    }
                }
            }
            free(words_chunk[i] -> string);
            free(words_chunk[i]);
        }
    } else {
        for (int i=0; i < max_words; i++) {
            if (i == 0) {
                local_words_chunk[i] = init_string(words_chunk[i] -> string);
                local_words_chunk[i] -> count = words_chunk[i] -> count;
                local_len_words_chunk = 1;
            } else {
                for (int j=0; j < local_len_words_chunk; j++) {
                    if (strlen(local_words_chunk[j] -> string) == strlen(words_chunk[i] -> string)) {
                        local_words_chunk[j] -> count += words_chunk[i] -> count;
                        break;
                    } else if (j == local_len_words_chunk - 1) {
                        local_words_chunk[j + 1] = init_string(words_chunk[i] -> string);
                        local_words_chunk[j + 1] -> count = words_chunk[i] -> count;
                        local_len_words_chunk++;
                        break;
                    }
                }
            }
            free(words_chunk[i] -> string);
            free(words_chunk[i]);
        }
    }
    free(words_chunk);
    shmWrite(shared_memory, local_words_chunk, local_len_words_chunk);
}

void *forkReduce(void **shared_memories, int *shared_index, long *identifiers, int type) {
    key_t key;
    int status, local_len_words_chunk;
    if (type == 1) {
        key = ftok(first_key_path, *shared_index);
    } else if (type == 2) {
        key = ftok(second_key_path, *shared_index);
    } else {
        key = ftok(third_key_path, *shared_index);
    }
    long identifier = shmget(key, sizeof(int) + sizeof(int) * MAXWORDS * (*shared_index) + sizeof(char) * MAXWORDLEN * MAXWORDS * (*shared_index) + sizeof(int) * MAXWORDS * (*shared_index), IPC_CREAT | SHM_W | SHM_R);
    identifiers[*shared_index] = identifier;
    void *local_memory = shmat(identifier, NULL, 0);
    pid_t child_process = fork();
    if (child_process == 0) {
        StringStruct **local_words_chunk = malloc(sizeof(StringStruct*) * MAXWORDS * (*shared_index));
        for (int i=0; i < *shared_index; i++) {
            void *shmaux = shared_memories[i];
            int *len_words_chunk = (int *) shmaux;
            shmaux = shmaux + sizeof(int);
            for (int j=0; j < *len_words_chunk; j++) {
                int *shmsize = (int *) shmaux;
                char *shmstring = malloc(sizeof(char) * (*shmsize + 1));
                strcpy(shmstring, (char *) (shmaux + sizeof(int)));
                int *shmcount = (int *) (shmaux + sizeof(int) + sizeof(char) * (*shmsize + 1));
                shmaux = (shmaux + sizeof(int) + sizeof(char) * (*shmsize + 1) + sizeof(int));
                if (i == 0 && j == 0) {
                    local_words_chunk[0] = init_string(shmstring);
                    local_words_chunk[0] -> count = *shmcount;
                    local_len_words_chunk = 1;
                } else {
                    if (type == 1) {
                        for (int k=0; k < local_len_words_chunk; k++) {
                            if (strcmp(local_words_chunk[k] -> string, shmstring) == 0) {
                                local_words_chunk[k] -> count += (*shmcount);
                                break;
                            } else if (k == local_len_words_chunk - 1) {
                                local_words_chunk[k + 1] = init_string(shmstring);
                                local_words_chunk[k + 1] -> count = (*shmcount);
                                local_len_words_chunk++;
                                break;
                            }
                        }
                    } else if (type == 2) {
                        for (int k=0; k < local_len_words_chunk; k++) {
                            if (*shmcount == local_words_chunk[k] -> count) {
                                int actual_string_size = strlen(local_words_chunk[k] -> string);
                                local_words_chunk[k] -> string = (char *) realloc(local_words_chunk[k] -> string, (actual_string_size + *shmsize + 2));
                                strcat(local_words_chunk[k] -> string, ";");
                                strcat(local_words_chunk[k] -> string, shmstring);
                                break;
                            } else if (k == local_len_words_chunk - 1) {
                                local_words_chunk[k + 1] = init_string(shmstring);
                                local_words_chunk[k + 1] -> count = (*shmcount);
                                local_len_words_chunk++;
                                break;
                            }
                        }
                    } else {
                        for (int k=0; k < local_len_words_chunk; k++) {
                            if (strlen(shmstring) == strlen(local_words_chunk[k] -> string)) {
                                local_words_chunk[k] -> count += (*shmcount);
                                break;
                            } else if (k == local_len_words_chunk - 1) {
                                local_words_chunk[k + 1] = init_string(shmstring);
                                local_words_chunk[k + 1] -> count = (*shmcount);
                                local_len_words_chunk++;
                                break;
                            }
                        }
                    }
                }
                free(shmstring);
            }
        }
        if (type == 1 || type == 3) {
            bubbleSort(local_words_chunk, local_len_words_chunk);
        }
        shmWrite(local_memory, local_words_chunk, local_len_words_chunk);
        free(identifiers);
        free(shared_memories);
        exit(0);
    } else {
        wait(&status);
        (*shared_index)++;
        for (int i=0; i < *shared_index; i++) {
            shmctl(identifiers[i], IPC_RMID, NULL);
        }
        *shared_index = 0;
    }
    return local_memory;
}

void forkMap(void *first_mapReduce_memory, void **shared_memories, long *identifiers, int *shared_index, int type) {
    pid_t wpid;
    int status;
    int index = 0;
    int childs_amount = 0;
    void *shmaux = first_mapReduce_memory;
    int *len_words_chunk = (int *) shmaux;
    shmaux = shmaux + sizeof(int);
    StringStruct **words_chunk = malloc(sizeof(StringStruct*) * (*len_words_chunk));
    for (int i=0; i < *len_words_chunk; i++) {
        int *shmsize = (int *) shmaux;
        char *shmstring = (char *) (shmaux + sizeof(int));
        int *shmcount = (int *) (shmaux + sizeof(int) + sizeof(char) * (*shmsize + 1));
        words_chunk[index] = init_string(shmstring);
        words_chunk[index] -> count = *shmcount;
        shmaux = (shmaux + sizeof(int) + sizeof(char) * (*shmsize + 1) + sizeof(int));
        index++;
        if (index == MAXWORDS) {
            index = 0;
            void *shared_memory = addSharedMemory(shared_memories, shared_index, identifiers, MAXWORDS, type);
            pid_t child_pid = fork();
            if (child_pid == 0) {  /* Child's code */
                shmStruct(shared_memory, words_chunk, MAXWORDS, type);
                free(identifiers);
                free(shared_memories);
                exit(0);
            } else {  /* Parent's code */
                childs_amount++;
                if (childs_amount == MAXCHILDS) {
                    wait(&status);
                    childs_amount--;
                }
                for (int j=0; j < MAXWORDS; j++) {
                    free(words_chunk[j] -> string);
                    free(words_chunk[j]);
                }
            }
        }
    }
    while ((wpid = wait(&status)) > 0);
    if (index != 0) {
        void *shared_memory = addSharedMemory(shared_memories, shared_index, identifiers, index, type);
        pid_t child_pid = fork();
        if (child_pid == 0) {  /* Child's code */
            shmStruct(shared_memory, words_chunk, index, type);
            free(identifiers);
            free(shared_memories);
            exit(0);
        } else {  /* Parent's code */
            wait(&status);
            for (int i=0; i < index; i++) {
                free(words_chunk[i] -> string);
                free(words_chunk[i]);
            }
            free(words_chunk);
        }
    }
}

void *threadMap(void *args) {
    MapArgs *aux_args = (MapArgs *) args;
    StringList *words_list = init_string_list(aux_args -> max_words);
    if (aux_args -> type == 1) {
        for (int i=0; i < aux_args -> max_words; i++) {
            if (i == 0) {
                words_list -> chunk[i] = init_string(aux_args -> words_chunk[i] -> string);
                words_list -> length = 1;
            } else {
                for (int j=0; j < words_list -> length; j++) {
                    if (strcmp(words_list -> chunk[j] -> string, aux_args -> words_chunk[i] -> string) == 0) {
                        words_list -> chunk[j] -> count++;
                        break;
                    } else if (j == words_list -> length - 1) {
                        words_list -> chunk[j + 1] = init_string(aux_args -> words_chunk[i] -> string);
                        words_list -> length++;
                        break;
                    }
                }
            }
        }
    } else if (aux_args -> type == 2) {
        for (int i=0; i < aux_args -> max_words; i++) {
            if (i == 0) {
                words_list -> chunk[i] = init_string(aux_args -> words_chunk[i] -> string);
                words_list -> chunk[i] -> count = aux_args -> words_chunk[i] -> count;
                words_list -> length = 1;
            } else {
                for (int j=0; j < words_list -> length; j++) {
                    if (words_list -> chunk[j] -> count == aux_args -> words_chunk[i] -> count) {
                        int actual_string_size = strlen(words_list -> chunk[j] -> string);
                        words_list -> chunk[j] -> string = 
                            (char *) realloc(words_list -> chunk[j] -> string, (actual_string_size + strlen(aux_args -> words_chunk[i] -> string) + 2));
                        strcat(words_list -> chunk[j] -> string, ";");
                        strcat(words_list -> chunk[j] -> string, aux_args -> words_chunk[i] -> string);
                        break;
                    } else if (j == words_list -> length - 1) {
                        words_list -> chunk[j + 1] = init_string(aux_args -> words_chunk[i] -> string);
                        words_list -> chunk[j + 1] -> count = aux_args -> words_chunk[i] -> count;
                        words_list -> length++;
                        break;
                    }
                }
            }
        }
    } else {
        for (int i=0; i < aux_args -> max_words; i++) {
            if (i == 0) {
                words_list -> chunk[i] = init_string(aux_args -> words_chunk[i] -> string);
                words_list -> chunk[i] -> count = aux_args -> words_chunk[i] -> count;
                words_list -> length = 1;
            } else {
                for (int j=0; j < words_list -> length; j++) {
                    if (strlen(words_list -> chunk[j] -> string) == strlen(aux_args -> words_chunk[i] -> string)) {
                        words_list -> chunk[j] -> count += aux_args -> words_chunk[i] -> count;
                        break;
                    } else if (j == words_list -> length - 1) {
                        words_list -> chunk[j + 1] = init_string(aux_args -> words_chunk[i] -> string);
                        words_list -> chunk[j + 1] -> count = aux_args -> words_chunk[i] -> count;
                        words_list -> length++;
                        break;
                    }
                }
            }
        }
    }
    pthread_exit((void *) words_list);
}

void *threadReduce(void *args) {
    ReduceArgs *aux_args = (ReduceArgs *) args;
    StringList *words_list = init_string_list(aux_args -> len_words_chunk_list * MAXWORDS);
    for (int i=0; i < aux_args -> len_words_chunk_list; i++) {
        StringList *words_chunk_list = aux_args -> words_chunk_list[i];
        for (int j=0; j < words_chunk_list -> length; j++) {
            if (i == 0 && j == 0) {
                words_list -> chunk[0] = init_string(words_chunk_list -> chunk[j] -> string);
                words_list -> chunk[0] -> count = words_chunk_list -> chunk[j] -> count;
                words_list -> length = 1;
            } else {
                if (aux_args -> type == 1) {
                    for (int k=0; k < words_list -> length; k++) {
                        if (strcmp(words_list -> chunk[k] -> string, words_chunk_list -> chunk[j] -> string) == 0) {
                            words_list -> chunk[k] -> count += words_chunk_list -> chunk[j] -> count;
                            break;
                        } else if (k == words_list -> length - 1) {
                            words_list -> chunk[k + 1] = init_string(words_chunk_list -> chunk[j] -> string);
                            words_list -> chunk[k + 1] -> count = words_chunk_list -> chunk[j] -> count;
                            words_list -> length++;
                            break;
                        }
                    }
                } else if (aux_args -> type == 2) {
                    for (int k=0; k < words_list -> length; k++) {
                        if (words_list -> chunk[k] -> count == words_chunk_list -> chunk[j] -> count) {
                            int actual_string_size = strlen(words_list -> chunk[k] -> string);
                            words_list -> chunk[k] -> string = (char *) realloc(words_list -> chunk[k] -> string, 
                                (actual_string_size + strlen(words_chunk_list -> chunk[j] -> string) + 2));
                            strcat(words_list -> chunk[k] -> string, ";");
                            strcat(words_list -> chunk[k] -> string, words_chunk_list -> chunk[j] -> string);
                            break;
                        } else if (k == words_list -> length - 1) {
                            words_list -> chunk[k + 1] = init_string(words_chunk_list -> chunk[j] -> string);
                            words_list -> chunk[k + 1] -> count = words_chunk_list -> chunk[j] -> count;
                            words_list -> length++;
                            break;
                        }
                    }
                } else {
                    for (int k=0; k < words_list -> length; k++) {
                        if (strlen(words_list -> chunk[k] -> string) == strlen(words_chunk_list -> chunk[j] -> string)) {
                            words_list -> chunk[k] -> count += words_chunk_list -> chunk[j] -> count;
                            break;
                        } else if (k == words_list -> length - 1) {
                            words_list -> chunk[k + 1] = init_string(words_chunk_list -> chunk[j] -> string);
                            words_list -> chunk[k + 1] -> count = words_chunk_list -> chunk[j] -> count;
                            words_list -> length++;
                            break;
                        }
                    }
                }
            }
        }
    }
    pthread_exit((void *) words_list);
}

int main(int argc, char* argv[]) {
    FILE *fp;
    int status;
    
    int type = 1;
    int index = 0;
    int shared_index = 0;
    int childs_amount = 0;
    int memory_buffer = 1;

    if (argc < 4) {
        printf("Usage: ./mapreduce <soure-file> <output-file> <version> [<type>]\n");
        return 1;
    }
    if (strcmp(argv[3], "fork") != 0 && strcmp(argv[3], "threads") != 0) {
        printf("Version must be 'fork' or 'threads'\n");
        return 1;
    }
    if (argc == 5) {
        type = strtol(argv[4], NULL, 10);
        if (type > 3 || type <= 0) {
            printf("[<type>] must be between 1 (default) and 3\n");
            return 1;
        }
    }
    if ((fp = fopen(argv[1], "r")) == NULL) {
        perror("fopen source-file");
        return 1;
    }

    if (strcmp(argv[3], "fork") == 0) { /* Fork */
        pid_t child_pid, wpid;
        char *word = malloc(sizeof(char) * MAXWORDLEN);
        StringStruct **words_chunk = malloc(sizeof(StringStruct*) * MAXWORDS);
        long *identifiers = malloc(sizeof(int) * memory_buffer);
        void **shared_memories = malloc(sizeof(void *) * memory_buffer);
        /* from 176 to 294 is the first MAPFORK */
        while(fscanf(fp, "%s", word) == 1) {
            words_chunk[index] = init_string(lowerString(word));
            index++;
            if (index == MAXWORDS) {
                index = 0;
                if (shared_index == memory_buffer) {
                    long *new_identifier = malloc(sizeof(long) * (memory_buffer * 2));
                    void **new_memories = malloc(sizeof(void *) * (memory_buffer * 2));
                    for (int i=0; i < memory_buffer; i++) {
                        new_identifier[i] = identifiers[i];
                        new_memories[i] = shared_memories[i];
                    }
                    memory_buffer *= 2;
                    free(identifiers);
                    free(shared_memories);
                    identifiers = new_identifier;
                    shared_memories = new_memories;
                }
                void *shared_memory = addSharedMemory(shared_memories, &shared_index, identifiers, MAXWORDS, 1);
                child_pid = fork();
                if (child_pid == 0) {  /* Child process */
                    shmStruct(shared_memory, words_chunk, MAXWORDS, 1);
                    free(word);
                    free(identifiers);
                    free(shared_memories);
                    fclose(fp);
                    exit(0);
                } else {  /* Parent Process */
                    childs_amount++;
                    if (childs_amount == MAXCHILDS) {
                        wait(&status);
                        childs_amount--;
                    }
                    for (int i=0; i < MAXWORDS; i++) {
                        free(words_chunk[i] -> string);
                        free(words_chunk[i]);
                    }
                }
            }
        }
        free(word);
        fclose(fp);
        while ((wpid = wait(&status)) > 0);

        if (index != 0) {  /* Last Chunk when index < MAXWORDS */
            if (shared_index == memory_buffer) {
                long *new_identifier = malloc(sizeof(long) * (memory_buffer * 2));
                void** new_memories = malloc(sizeof(void *) * (memory_buffer * 2));
                for (int i=0; i < memory_buffer; i++) {
                    new_identifier[i] = identifiers[i];
                    new_memories[i] = shared_memories[i];
                }
                memory_buffer *= 2;
                free(identifiers);
                free(shared_memories);
                identifiers = new_identifier;
                shared_memories = new_memories;
            }
            void *shared_memory = addSharedMemory(shared_memories, &shared_index, identifiers, index, 1);
            child_pid = fork();
            if (child_pid == 0) {  /* Child's code */
                shmStruct(shared_memory, words_chunk, index, 1);
                free(identifiers);
                free(shared_memories);
                exit(0);
            } else {  /* Parent's code */
                wait(&status);
                for (int i=0; i < index; i++) {
                    free(words_chunk[i] -> string);
                    free(words_chunk[i]);
                }
            }
        }

        free(words_chunk);

        void *local_memory = forkReduce(shared_memories, &shared_index, identifiers, 1);
        shared_index = 0;
        free(identifiers);
        free(shared_memories);
        memory_buffer = *((int *) local_memory) / MAXWORDS + 2;
        identifiers = malloc(sizeof(long) * memory_buffer);
        shared_memories = malloc(sizeof(void*) * memory_buffer);
        forkMap(local_memory, shared_memories, identifiers, &shared_index, 2);
        void *second_local_memory = forkReduce(shared_memories, &shared_index, identifiers, 2);
        
        shared_index = 0;
        free(identifiers);
        free(shared_memories);
        memory_buffer = *((int *) local_memory) / MAXWORDS + 2;
        identifiers = malloc(sizeof(long) * memory_buffer);
        shared_memories = malloc(sizeof(void*) * memory_buffer);
        forkMap(local_memory, shared_memories, identifiers, &shared_index, 3);
        void *third_local_memory = forkReduce(shared_memories, &shared_index, identifiers, 3);

        free(identifiers);
        free(shared_memories);

        void *shmaux;
        FILE *fout = fopen(argv[2], "w");
        if (type == 1) {
            fprintf(fout, "repeticiones,palabra\n");
            shmaux= local_memory;
        } else if (type == 2) {
            fprintf(fout, "repeticiones,palabras\n");
            shmaux = second_local_memory;
        } else {
            fprintf(fout, "cantidad,palabras\n");
            shmaux = third_local_memory;
        }
        int *shmlen = (int *) shmaux;
        shmaux = shmaux + sizeof(int);
        for (int i=0; i < *shmlen; i++) {
            int *shmsize = (int *) shmaux;
            char *shmstring = (char *) (shmaux + sizeof(int));
            int *shmcount = (int *) (shmaux + sizeof(int) + sizeof(char) * (*shmsize + 1));
            if (type == 1) {
                fprintf(fout, "%i,%s\n", *shmcount, shmstring);
            } else if (type == 2) {
                fprintf(fout, "%i,[%s]\n", *shmcount, shmstring);
            } else {
                fprintf(fout, "%i,%lu\n", *shmcount, strlen(shmstring));
            }
            shmaux = (shmaux + sizeof(int) + sizeof(char) * (*((int *) shmaux) + 1) + sizeof(int));
        }
        fclose(fout);
    } else {  /* threads */
        void *retorno;
        char *word = malloc(sizeof(char) * MAXWORDLEN);
        pthread_t *thread_ids = malloc(sizeof(pthread_t));
        StringList **words_chunk_list = malloc(sizeof(StringList*));
        StringStruct **words_chunk = malloc(sizeof(StringStruct*) * MAXWORDS);
        while(fscanf(fp, "%s", word) == 1) {
            words_chunk[index] = init_string(lowerString(word));
            index++;
            if (index == MAXWORDS) {
                index = 0;
                if (childs_amount == memory_buffer) {
                    thread_ids = (pthread_t *) realloc(thread_ids, sizeof(pthread_t) * memory_buffer * 2);
                    words_chunk_list = (StringList **) realloc(words_chunk_list, sizeof(StringList *) * memory_buffer * 2);
                    memory_buffer *= 2;
                }
                MapArgs *args = init_MapArgs(1, MAXWORDS, childs_amount, words_chunk);
                pthread_create(&thread_ids[childs_amount], NULL, threadMap, (void *) args);
                pthread_join(thread_ids[childs_amount], &retorno);
                words_chunk_list[childs_amount] = (StringList *) retorno;
                childs_amount++;
                for (int i=0; i < MAXWORDS; i++) {
                    free(words_chunk[i] -> string);
                    free(words_chunk[i]);
                }
                free(args);
            }
        }
        free(word);
        fclose(fp);
        if (index != 0) {
            if (childs_amount == memory_buffer) {
                thread_ids = (pthread_t *) realloc(thread_ids, sizeof(pthread_t) * memory_buffer * 2);
                memory_buffer *= 2;
            }
            MapArgs *args = init_MapArgs(1, index, childs_amount, words_chunk);
            pthread_create(&thread_ids[childs_amount], NULL, threadMap, (void *) args);
            pthread_join(thread_ids[childs_amount], &retorno);
            words_chunk_list[childs_amount] = (StringList *) retorno;
            childs_amount++;
            for (int i=0; i < index; i++) {
                free(words_chunk[i] -> string);
                free(words_chunk[i]);
            }
            free(args);
        }

        /* First reduce */
        ReduceArgs *first_ReduceArgs = init_ReduceArgs(1, childs_amount, words_chunk_list);
        pthread_create(&thread_ids[childs_amount], NULL, threadReduce, (void *) first_ReduceArgs);
        pthread_join(thread_ids[childs_amount], &retorno);
        StringList *first_reduced_chunk = (StringList*) retorno;
        bubbleSort(first_reduced_chunk -> chunk, first_reduced_chunk -> length);
        for (int i=0; i < childs_amount; i++) {
            for (int j=0; j < words_chunk_list[i] -> length; j++) {
                free(words_chunk_list[i] -> chunk[j] -> string);
                free(words_chunk_list[i] -> chunk[j]);
            }
            free(words_chunk_list[i]);
        }
        free(thread_ids);
        free(first_ReduceArgs);
        free(words_chunk_list);
        memory_buffer = 1;
        childs_amount = 0;
        index = 0;

        /* Second Map */
        thread_ids = malloc(sizeof(pthread_t));
        words_chunk_list = malloc(sizeof(StringList*));
        for (int i=0; i < first_reduced_chunk -> length; i++) {
            words_chunk[index] = first_reduced_chunk -> chunk[i];
            index++;
            if (index == MAXWORDS) {
                index = 0;
                if (childs_amount == memory_buffer) {
                    thread_ids = (pthread_t *) realloc(thread_ids, sizeof(pthread_t) * memory_buffer * 2);
                    words_chunk_list = (StringList **) realloc(words_chunk_list, sizeof(StringList *) * memory_buffer * 2);
                    memory_buffer *= 2;
                }
                MapArgs *second_MapArgs = init_MapArgs(2, MAXWORDS, childs_amount, words_chunk);
                pthread_create(&thread_ids[childs_amount], NULL, threadMap, (void *) second_MapArgs);
                pthread_join(thread_ids[childs_amount], &retorno);
                words_chunk_list[childs_amount] = (StringList *) retorno;
                childs_amount++;
                free(second_MapArgs);
            }
        }
        if (index != 0) {
            if (childs_amount == memory_buffer) {
                thread_ids = (pthread_t *) realloc(thread_ids, sizeof(pthread_t) * memory_buffer * 2);
                memory_buffer *= 2;
            }
            MapArgs *second_MapArgs = init_MapArgs(2, index, childs_amount, words_chunk);
            pthread_create(&thread_ids[childs_amount], NULL, threadMap, (void *) second_MapArgs);
            pthread_join(thread_ids[childs_amount], &retorno);
            words_chunk_list[childs_amount] = (StringList *) retorno;
            childs_amount++;
            free(second_MapArgs);
        }

        /* Second Reduce */
        ReduceArgs *second_ReduceArgs = init_ReduceArgs(2, childs_amount, words_chunk_list);
        pthread_create(&thread_ids[childs_amount], NULL, threadReduce, (void *) second_ReduceArgs);
        pthread_join(thread_ids[childs_amount], &retorno);
        StringList *second_reduced_chunk = (StringList *) retorno;
        for (int i=0; i < childs_amount; i++) {
            for (int j=0; j < words_chunk_list[i] -> length; j++) {
                free(words_chunk_list[i] -> chunk[j] -> string);
                free(words_chunk_list[i] -> chunk[j]);
            }
            free(words_chunk_list[i]);
        }
        free(thread_ids);
        free(words_chunk_list);
        free(second_ReduceArgs);
        memory_buffer = 1;
        childs_amount = 0;
        index = 0;

        /* Third Map */
        thread_ids = malloc(sizeof(pthread_t));
        words_chunk_list = malloc(sizeof(StringList*));
        for (int i=0; i < first_reduced_chunk -> length; i++) {
            words_chunk[index] = first_reduced_chunk -> chunk[i];
            index++;
            if (index == MAXWORDS) {
                index = 0;
                if (childs_amount == memory_buffer) {
                    thread_ids = (pthread_t *) realloc(thread_ids, sizeof(pthread_t) * memory_buffer * 2);
                    words_chunk_list = (StringList **) realloc(words_chunk_list, sizeof(StringList *) * memory_buffer * 2);
                    memory_buffer *= 2;
                }
                MapArgs *third_MapArgs = init_MapArgs(3, MAXWORDS, childs_amount, words_chunk);
                pthread_create(&thread_ids[childs_amount], NULL, threadMap, (void *) third_MapArgs);
                pthread_join(thread_ids[childs_amount], &retorno);
                words_chunk_list[childs_amount] = (StringList *) retorno;
                childs_amount++;
                free(third_MapArgs);
            }
        }
        if (index != 0) {
            if (childs_amount == memory_buffer) {
                thread_ids = (pthread_t *) realloc(thread_ids, sizeof(pthread_t) * memory_buffer * 2);
                memory_buffer *= 2;
            }
            MapArgs *third_MapArgs = init_MapArgs(3, index, childs_amount, words_chunk);
            pthread_create(&thread_ids[childs_amount], NULL, threadMap, (void *) third_MapArgs);
            pthread_join(thread_ids[childs_amount], &retorno);
            words_chunk_list[childs_amount] = (StringList *) retorno;
            childs_amount++;
            free(third_MapArgs);
        }

        /* Third Reduce */
        ReduceArgs *third_ReduceArgs = init_ReduceArgs(3, childs_amount, words_chunk_list);
        pthread_create(&thread_ids[childs_amount], NULL, threadReduce, (void *) third_ReduceArgs);
        pthread_join(thread_ids[childs_amount], &retorno);
        StringList *third_reduced_chunk = (StringList *) retorno;
        bubbleSort(third_reduced_chunk -> chunk, third_reduced_chunk -> length);
        for (int i=0; i < childs_amount; i++) {
            for (int j=0; j < words_chunk_list[i] -> length; j++) {
                free(words_chunk_list[i] -> chunk[j] -> string);
                free(words_chunk_list[i] -> chunk[j]);
            }
            free(words_chunk_list[i]);
        }
        free(thread_ids);
        free(words_chunk);
        free(third_ReduceArgs);
        free(words_chunk_list);

        /* Output */
        StringList *list;
        FILE *fout = fopen(argv[2], "w");
        if (type == 1) {
            fprintf(fout, "repeticiones,palabra\n");
            list = first_reduced_chunk;
        } else if (type == 2) {
            fprintf(fout, "repeticiones,palabras\n");
            list = second_reduced_chunk;
        } else {
            fprintf(fout, "cantidad,palabras\n");
            list = third_reduced_chunk;
        }
        for (int i=0; i < list -> length; i++) {
            if (type == 1) {
                fprintf(fout, "%i,%s\n", list -> chunk[i] -> count, list -> chunk[i] -> string);
            } else if (type == 2) {
                fprintf(fout, "%i,[%s]\n", list -> chunk[i] -> count, list -> chunk[i] -> string);
            } else {
                fprintf(fout, "%i,%lu\n", list -> chunk[i] -> count, strlen(list -> chunk[i] -> string));
            }
        }
        fclose(fout);

        for (int i=0; i < second_reduced_chunk -> length; i++) {
            free(second_reduced_chunk -> chunk[i] -> string);
            free(second_reduced_chunk -> chunk[i]);
        }
        for (int i=0; i < third_reduced_chunk -> length; i++) {
            free(third_reduced_chunk -> chunk[i] -> string);
            free(third_reduced_chunk -> chunk[i]);
        }
        for (int i=0; i < first_reduced_chunk -> length; i++) {
            free(first_reduced_chunk -> chunk[i] -> string);
            free(first_reduced_chunk -> chunk[i]);
        }
        free(first_reduced_chunk);
    }
    return 0;
}
