//  event.h
//
//  SOPE Project 2 - Table for Playing Cards
//
//  Turma 1 Grupo 5
//
//  ei11089 - Diogo Santos
//  ei11122 - Pedro Fernandes
//

#ifndef EVENT_H
#define	EVENT_H

#include "card.h"
#include <time.h>
#include <string.h>

// read
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// write
#include <unistd.h>

#include <errno.h>

#define MAX_LEN 1024
#define MAX_EVENTS 1000
#define LOG_FILE ".log"

#define LOG_COLUMN_FORMAT "%-20s | %-20s | %-20s | %s\n"

/**
 * An event represents all possible actions in a game of cards for easier logging of these actions.
 */
typedef struct {
    
    /**
     * Indicates player who created the event. If it's a deal event it should say Dealer-DealerName.
     * If it's table or gameover, it should just say '-'.
     */
    unsigned int playerID;
    
    /** A string representing the player name. */
    char playerName[MAX_LEN];
    
    /**
     * There are 5 possible types of events: deal, receive_hand, play, hand, gameover.
     */
    char eventType[MAX_LEN];
    
    /**
     * This indicates the cards relating to a result. 
     * If deal or gameover, just a '-' will be printed (empty array).
     */
    card_t result[DECK_CARDS];
    
    /** Size of the result card array. */
    unsigned int numberOfCardsInResult;
    
    /** This is the time stamp of the event. */
    time_t timeStamp;
    
} event_t;

/**
 * Puts the event information into a string.
 * @param event The event to be represented.
 * @return The formatted string with the event information.
 */
char* get_event_representation(event_t event);

/**
 * Prints an event to the screen, in a formatted fashion.
 * @param event The event to be printed.
 */
void print_event(event_t event);

/**
 * Prints an array of events in a formatted fashion.
 * @param events The events to be displayed.
 * @param numberOfEvents The size of the events array.
 */
void print_event_list(event_t events[], int numberOfEvents);

/**
 * Prints an array of events in a formatted fashion.
 * This will take into account other players, so that only the current player can see the information about their hand and not the hands of other players.
 * @param events The events to be displayed.
 * @param numberOfEvents The size of the events array.
 */
void print_event_list_for_player(event_t events[], int numberOfEvents);

/**
 * Creates a file for logging information about a game.
 * @param logname The name of the log file to be created.
 */
void initialize_event_log(char * logname);

/**
 * Writes an event to the log file of the game.
 * @param logname The log file name of the current game.
 */
void write_to_log(char * logname, event_t event);

#endif	/* EVENT_H */

