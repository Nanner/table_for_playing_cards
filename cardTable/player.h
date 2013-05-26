//  player.h
//
//  SOPE Project 2 - Table for Playing Cards
//
//  Turma 1 Grupo 5
//
//  ei11089 - Diogo Santos
//  ei11122 - Pedro Fernandes
//

#ifndef PLAYER_H
#define	PLAYER_H

#include <stdio.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include "card.h"

#define MAX_STRING_LENGTH 1024
#define MAXIMUM_NUMBER_OF_PLAYERS 10
#define HAND_CARDS 5

/**
 * The player struct represents a player's id, nickname and fifoname.
 */
typedef struct {
    unsigned int id; /**< The player's ID */
    char nickname[MAX_STRING_LENGTH]; /**< The player's nickname*/
    char fifoName[MAX_STRING_LENGTH]; /**< The player's fifoName (related to his nickname and table it's on) */
} player_t;

/**
 * Creates a FIFO for the player and opens it for read.
 * @param fifoName The name of the FIFO to create
 * @return The FIFO file descriptor, if successful, -1 otherwise.
 */
int create_player_fifo(char* fifoName);

/**
 * Opens a player FIFO for write.
 * @param fifoName The name of the FIFO to open
 * @return The FIFO file descriptor, if successful, -1 otherwise.
 */
int open_player_fifo(char* fifoName);

/**
 * Closes and destroys the player FIFO.
 * @param fd File descriptor of the FIFO to be closed
 * @param fifoName Name of the FIFO to be closed
 * @return 0 if successful, -1 otherwise.
 */
int close_player_fifo(int fd, char* fifoName);

/**
 * Receives a card hand from the player FIFO, saving it on cardHand.
 * @param fd File descriptor for the player FIFO
 * @param cardHand Hand that receives the FIFO cards
 * @param numberOfCards Number of cards to receive.
 */
void get_hand_from_fifo(int fd, card_t cardHand[], int numberOfCards);

/**
 * Sends a card hand from cardHand to the player FIFO.
 * @param fd File descriptor for the player FIFO
 * @param cardHand Hand to be sent to the FIFO
 * @param numberOfCards Number of cards to send.
 */
void send_hand_to_fifo(char* fifoName, card_t cardHand[], int numberOfCards);

#endif	/* PLAYER_H */

