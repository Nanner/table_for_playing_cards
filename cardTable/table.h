/* 
 * File:   table.h
 * Author: nanner
 *
 * Created on May 9, 2013, 6:59 PM
 */

#ifndef TABLE_H
#define	TABLE_H

#include "player.h"
#include "card.h"

typedef struct {
    /* Shared Variables */
    player_t players[10];
    unsigned int numberOfPlayers;
    unsigned int currentTurn;
    unsigned int roundNumber;
    unsigned int dealerID;
    card_t deck[MAXIMUM_CARDS];
    card_t tableCards[MAXIMUM_CARDS];
    
    /* Sync Variables*/
    //TODO Mutexes and stuff
} table_t;

#endif	/* TABLE_H */

