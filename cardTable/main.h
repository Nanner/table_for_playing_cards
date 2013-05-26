//  main.h
//
//  SOPE Project 2 - Table for Playing Cards
//
//  Turma 1 Grupo 5
//
//  ei11089 - Diogo Santos
//  ei11122 - Pedro Fernandes
//

#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include "table.h"

#define TABLE_READY_SEM "/table_ready"
#define MAX_LEN 1024

static int gameState = CONTINUE_GAME; /**< Indicates current game state (continue or stop)*/

static player_t player; /**< User's player variable */
static bool isDealer = false; /**< Indicates if this user is Dealer or not */

static char semaphoreName[MAX_LEN]; /**< Name of the semaphore used in initial synchronizations */
static char tableName[MAX_LEN]; /**< Name of this game's table */
static table_t* table; /**< This game's table (shared memory) */
static sem_t* table_ready; /**< The semaphore used in initial synchronizations */
static pthread_mutex_t syncMut = PTHREAD_MUTEX_INITIALIZER; /**< Mutex used for intra-process synchronization */
static int fifoFD; /**< File descriptor for local FIFO */
static char fifoName[MAX_LEN]; /**< Name of local FIFO */
static pthread_t tidSync; /**< Game synchronization thread ID */
static pthread_t tidInterface; /**< User interface thread ID */

static int playersAwaited; /**< Number of players expected in this table */
static unsigned int currentTurn; /**< Current turn of the game */
static unsigned int roundNumber; /**< Current round of the game */

static card_t tableCards[DECK_CARDS]; /**< Cards currently on the table */
static unsigned int numberOfCardsOnTable; /**< Number of cards currently on the table */

static card_t handCards[HAND_CARDS]; /**< Cards on the player hand */

/**
 * Prompts the user with the string in prompt to pick an option from 0 to maxChoice, returning it. Verifies if
 * the input is valid.
 * @param prompt The string to be prompted to the user.
 * @param maxChoice The maximum choice allowed.
 * @return The choice the user made.
 */
int getChoice(const char *prompt, int maxChoice);

/**
 * Adds an event to the event array on the shared memory. Locks and unlocks the necessary mutexes.
 * @param eventType A string indicating the type of event.
 * @param result A card_t array containing the event "result", that is, the cards relating to that event.
 * @param numberOfCardsInResult The number of cards result array contains.
 */
void add_event(char* eventType, card_t result[], int numberOfCardsInResult);

/**
 * Adds an event to the event array on the shared memory. This event has no result card array. Locks and unlocks necessary mutexes.
 * @param eventType A string indicating the type of event.
 */
void add_event_without_result(char* eventType);

/**
 * Gives HAND_CARDS (defined in player.h) cards to each player in the game.
 * Card hands are sent to the respective player FIFO's.
 */
void deal_cards();

/**
 * Prints the time elapsed since the beginning of the current round. Locks and unlocks the necessary mutexes to avoid
 * synchronization problems.
 */
void check_turn_elapsed_time();

/**
 * Prints the time elapsed since the beginning of the current turn. Locks and unlocks the necessary mutexes to avoid
 * synchronization problems.
 */
void check_round_elapsed_time();

/**
 * Shows the player his current cards and asks for a card to play. After play, mutexes are locked and variables are updated.
 * After that, a turn change is broadcasted to the other processes.
 */
void play_card();

/**
 * Initializes the global variables local to this process.
 */
void initialize_local_variables();

/**
 * This function is meant to be run on a different thread. Regulates player interactions and avoids busy waiting on game
 * synchronization as much as possible. Shows the main menu of the game and provides the player with ways to call the
 * application functions.
 * @param arg This is not used.
 * @return NULL when the function terminates.
 */
void* game_interface(void* arg);

/**
 * This function is meant to be run on a different thread. Synchronizes the process with the shared memory on each turn change
 * of the game (waits on a turn change condition variable). Also notifies the user of turn and round changes, and cards played.
 * @param arg This is not used.
 * @return NULL when the funtion terminates.
 */
void* game_sync(void* arg);



#endif
