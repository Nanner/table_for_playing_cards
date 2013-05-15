/* 
 * File:   player.h
 * Author: nanner
 *
 * Created on May 9, 2013, 7:08 PM
 */

#ifndef PLAYER_H
#define	PLAYER_H

#include <stdio.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>
#include "card.h"

#define MAX_STRING_LENGTH 1024
#define MAXIMUM_NUMBER_OF_PLAYERS 10
#define HAND_CARDS 5

typedef struct {
    unsigned int id;
    char nickname[MAX_STRING_LENGTH];
    char fifoName[MAX_STRING_LENGTH];
    //TODO card array for hand?
} player_t;

int create_player_fifo(char* fifoName);

int open_player_fifo(char* fifoName);

void get_hand_from_fifo(int fd, card_t cardHand[]);

void send_hand_to_fifo(int fd, card_t cardHand[], int numberOfCards);

#endif	/* PLAYER_H */

