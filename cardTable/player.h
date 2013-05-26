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

typedef struct {
    unsigned int id;
    char nickname[MAX_STRING_LENGTH];
    char fifoName[MAX_STRING_LENGTH];
    //TODO card array for hand?
} player_t;

int create_player_fifo(char* fifoName);

int open_player_fifo(char* fifoName);

int close_player_fifo(int fd, char* fifoName);

void get_hand_from_fifo(int fd, card_t cardHand[], int numberOfCards);

void send_hand_to_fifo(char* fifoName, card_t cardHand[], int numberOfCards);

#endif	/* PLAYER_H */

