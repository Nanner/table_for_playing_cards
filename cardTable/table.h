//  table.h
//
//  SOPE Project 2 - Table for Playing Cards
//
//  Turma 1 Grupo 5
//
//  ei11089 - Diogo Santos
//  ei11122 - Pedro Fernandes
//

#ifndef TABLE_H
#define	TABLE_H

#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>

#include "player.h"
#include "card.h"
#include "event.h"

#define CONTINUE_GAME 1
#define STOP_GAME 0

typedef struct {
    /* Shared Variables */
    unsigned int gameState;
    event_t eventList[MAX_EVENTS];
    unsigned int numberOfEvents;
    
    unsigned int playersAwaited;
    player_t players[10];
    unsigned int numberOfPlayers;
    
    time_t turnStartTimestamp;
    unsigned int currentTurn;
    time_t roundStartTimestamp;
    unsigned int roundNumber;
    
    unsigned int dealerID;
    unsigned int firstPlayerID;
    
    unsigned int cardsDealt;
    card_t deck[DECK_CARDS];
    
    card_t tableCards[DECK_CARDS];
    unsigned int numberOfCardsOnTable;

    /* Sync Variables*/
    pthread_mutex_t dealingCardsLock;
    pthread_cond_t dealingCardsCond;

    pthread_mutex_t tableAccessLock;
    pthread_cond_t turnChangeCond;
    
    pthread_mutex_t playerWaitLock;
    pthread_cond_t playerWaitCond;
    
    pthread_mutex_t addingEventLock;
} table_t;

table_t* create_table(char* tableName, int tableSize);

table_t* attach_table(char* tableName, int tableSize);

bool init_sync_variables_in_table(table_t* table);

void destroy_table(table_t* table, char* tableName, int tableSize);

bool check_if_player_exists(table_t* table, char* playerName);

#endif	/* TABLE_H */

