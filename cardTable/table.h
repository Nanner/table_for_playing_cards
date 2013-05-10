/* 
 * File:   table.h
 * Author: nanner
 *
 * Created on May 9, 2013, 6:59 PM
 */

#ifndef TABLE_H
#define	TABLE_H

#include <fcntl.h>
#include <sys/mman.h>

#include "player.h"
#include "card.h"

typedef struct {
    /* Shared Variables */
    player_t players[10];
    unsigned int numberOfPlayers;
    unsigned int currentTurn;
    unsigned int roundNumber;
    unsigned int dealerID;
    card_t deck[DECK_CARDS];
    card_t tableCards[DECK_CARDS];
    
    /* Sync Variables*/
    //TODO Mutexes and stuff
} table_t;

table_t* create_table(char* tableName, int tableSize);

table_t* attach_table(char* tableName, int tableSize);

void destroy_table(table_t* table, char* tableName, int tableSize);

#endif	/* TABLE_H */

