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
#define MAX_PLAYERS 100

/**
 * The game table structure
 * This structure shall be shared with several processes
 * over shared memory.
 * Contains all the game variables and the mutexes and condition variables
 * to be used.
 */
typedef struct {
    
    /* Shared Variables */
    unsigned int gameState; /**< Current game state (continue or stop) */
    event_t eventList[MAX_EVENTS]; /**< An array saving all the game events */
    unsigned int numberOfEvents; /**< Current number of events in the event array */
    
    unsigned int playersAwaited; /**< Name of players expected to play in the table */
    player_t players[MAX_PLAYERS]; /**< An array containing the game's players */
    unsigned int numberOfPlayers; /**< Number of players in the game */
    
    time_t turnStartTimestamp; /**< Saves the time the turn started at */
    unsigned int currentTurn; /**< Current game turn */
    time_t roundStartTimestamp; /**< Saves the time the round started at */
    unsigned int roundNumber;  /**< Current game round */
    
    unsigned int dealerID; /**< Dealer ID */
    unsigned int firstPlayerID; /**< ID of the first player to play */
    
    unsigned int cardsDealt; /**< Indicates if cards have been dealt or not */
    card_t deck[DECK_CARDS]; /**< Game's card deck */
    
    card_t tableCards[DECK_CARDS]; /**< Cards on the table */
    unsigned int numberOfCardsOnTable; /**< The number of cards currently on the table */

    /* Sync Variables*/
    pthread_mutex_t dealingCardsLock; /**< This mutex prevents access while cards are being dealt */
    pthread_cond_t dealingCardsCond; /**< This condition variable waits for cards to be dealt */

    pthread_mutex_t tableAccessLock; /**< This mutex prevents simultaneous access to the table while members are being accessed */
    pthread_cond_t turnChangeCond; /**< This condition variable waits for a turn change to be broadcasted */
    
    pthread_mutex_t playerWaitLock; /**< This mutex is related to waiting for enough players to join the table */
    pthread_cond_t playerWaitCond; /**< This condition variable waits for all players to enter the game */
    
    pthread_mutex_t addingEventLock; /**< This mutex prevents access while an event is being added to the event array */
    
} table_t;

/**
 * Creates a table_t variable, allocating it to the shared memory, and initializes its variables.
 * @param tableName The name of the table (shared memory)
 * @param tableSize The size of the table (shared memory)
 * @return A pointer to the table_t created, if successful, NULL if not.
 */
table_t* create_table(char* tableName, int tableSize);

/**
 * Tries to open the shared memory and return a pointer to that table_t variable.
 * @param tableName The name of the table (shared memory)
 * @param tableSize The size of the table (shared memory)
 * @return A pointer to the table_t opened, if successful, NULL if not.
 */
table_t* attach_table(char* tableName, int tableSize);

/**
 * Initializes the mutexes and condition variables on the table.
 * @param table Pointer to the table to initialize the elements on.
 * @return True if initialization was successful, false if not.
 */
bool init_sync_variables_in_table(table_t* table);

/**
 * Unmaps and unlinks the table (shared memory).
 * @param table Pointer to the table_t being destroyed
 * @param tableName The table name (shared memory)
 * @param tableSize The table size (shared memory)
 */
void destroy_table(table_t* table, char* tableName, int tableSize);

/**
 * Checks if a player with the name given exists in the current table.
 * @param table The table to check on
 * @param playerName The name of the player to look for
 * @return True if the player exists, false otherwise.
 */
bool check_if_player_exists(table_t* table, char* playerName);

#endif	/* TABLE_H */

