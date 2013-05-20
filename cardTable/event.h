/* 
 * File:   event.h
 * Author: nanner
 *
 * Created on May 20, 2013, 1:39 AM
 */

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
#define LOG_FILE ".txt"

typedef struct {
    
    // Indicates player who created the event. If it's a deal event it should say Dealer-DealerName.
    // If it's table or gameover, it should just say '-'.
    unsigned int playerID;
    char playerName[MAX_LEN];
    
    // There are 5 possible events: deal, receive_hand, play, hand, gameover
    // Hand should always appear after a play event
    char eventType[MAX_LEN];
    
    // This indicates the cards relating to result. If deal or gameover, just a '-' will be printed (empty array)
    card_t result[DECK_CARDS];
    unsigned int numberOfCardsInResult;
    
    // This is the time stamp of the event
    time_t timeStamp;
    
} event_t;

char* get_event_representation(event_t event);

void print_event(event_t event);

void print_event_list(event_t events[], int numberOfEvents);

void initialize_event_log(char * logname);

void write_to_log(char * logname, event_t event);

#endif	/* EVENT_H */

