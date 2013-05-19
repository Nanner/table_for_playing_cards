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
    table->gameState = CONTINUE_GAME;
    table->numberOfPlayers = 1;
    table->playersAwaited = 0;
    table->currentTurn = 0;
    table->roundNumber = 0;
    table->dealerID = 0;
    table->firstPlayerID = 0;
    table->cardsDealt = 0;
    table->numberOfCardsOnTable = 0;
    start_deck(table->deck);
    initialize_with_usedCard(table->tableCards, DECK_CARDS);

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

bool init_sync_variables_in_table(table_t* table) {
    pthread_mutexattr_t tableAccessAttr;
    
    if(pthread_mutexattr_init(&tableAccessAttr) != 0)
        return false;
    
    if(pthread_mutexattr_setpshared(&tableAccessAttr, PTHREAD_PROCESS_SHARED) != 0)
        return false;
    
    if(pthread_mutex_init(&table->tableAccessLock, &tableAccessAttr) != 0)
        return false;
    
    pthread_condattr_t turnChangeAttr;
    
    if(pthread_condattr_init(&turnChangeAttr) != 0)
        return false;
    
    if(pthread_condattr_setpshared(&turnChangeAttr, PTHREAD_PROCESS_SHARED) != 0)
        return false;
    
    if(pthread_cond_init(&table->turnChangeCond, &turnChangeAttr) != 0)
        return false;
    
    pthread_mutexattr_t playerWaitAttr;
    
    if(pthread_mutexattr_init(&playerWaitAttr) != 0)
        return false;
    
    if(pthread_mutexattr_setpshared(&playerWaitAttr, PTHREAD_PROCESS_SHARED) != 0)
        return false;
    
    if(pthread_mutex_init(&table->playerWaitLock, &playerWaitAttr) != 0)
        return false;
    
    pthread_condattr_t playerWaitCondAttr;
    
    if(pthread_condattr_init(&playerWaitCondAttr) != 0)
        return false;
    
    if(pthread_condattr_setpshared(&playerWaitCondAttr, PTHREAD_PROCESS_SHARED) != 0)
        return false;
    
    if(pthread_cond_init(&table->playerWaitCond, &playerWaitCondAttr) != 0)
        return false;
    
    pthread_mutexattr_t dealingCardsLockAttr;
    
    if(pthread_mutexattr_init(&dealingCardsLockAttr) != 0)
        return false;
    
    if(pthread_mutexattr_setpshared(&dealingCardsLockAttr, PTHREAD_PROCESS_SHARED) != 0)
        return false;
    
    if(pthread_mutex_init(&table->dealingCardsLock, &dealingCardsLockAttr) != 0)
        return false;
    
    pthread_condattr_t dealingCardsCondAttr;
    
    if(pthread_condattr_init(&dealingCardsCondAttr) != 0)
        return false;
    
    if(pthread_condattr_setpshared(&dealingCardsCondAttr, PTHREAD_PROCESS_SHARED) != 0)
        return false;
    
    if(pthread_cond_init(&table->dealingCardsCond, &dealingCardsCondAttr) != 0)
        return false;
    
    return true;
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
