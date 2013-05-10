#include "table.h"

table_t* create_table(char* tableName, int tableSize) {
    int tableFD;
    table_t* table;

    tableFD = shm_open(tableName, O_CREAT | O_RDWR, 0660);

    if (tableFD < 0) {
        perror("Failure on table %s creation (shm_open())\n");
        return NULL;
    }

    //specify the size of the shared memory region the table will be allocated on
    if (ftruncate(tableFD, tableSize) < 0) {
        perror("Failure specifying size on ftruncate()\n");
        return NULL;
    }

    //attach this region to virtual memory
    table = mmap(0, tableSize, PROT_READ | PROT_WRITE, MAP_SHARED, tableFD, 0);
    if (table == MAP_FAILED) {
        perror("Failure in mmap()");
        return NULL;
    }

    //initialize data in the table structure
    table->numberOfPlayers = 1;
    table->currentTurn = 0;
    table->roundNumber = 0;
    table->dealerID = 0;

    return (table_t *) table;
}

table_t* attach_table(char* tableName, int tableSize) {

    int tableFD;
    table_t* table;
    tableFD = shm_open(tableName, O_RDWR, 0660);
    if (tableFD < 0) {
        perror("Failure opening table");
        return NULL;
    }
    
    //attach this region to virtual memory 
    table = mmap(0, tableSize, PROT_READ | PROT_WRITE, MAP_SHARED, tableFD, 0);
    if (table == MAP_FAILED) {
        perror("Failure in mmap()");
        return NULL;
    }
    return (table_t *) table;
}

void destroy_table(table_t* table, char* tableName, int tableSize) {
    if (munmap(table, tableSize) < 0) {
        perror("Failure in munmap()");
        exit(EXIT_FAILURE);
    }

    if (shm_unlink(tableName) < 0) {
        perror("Failure in shm_unlink()");
        exit(EXIT_FAILURE);
    }
}
