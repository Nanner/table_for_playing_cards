/* 
 * File:   player.h
 * Author: nanner
 *
 * Created on May 9, 2013, 7:08 PM
 */

#ifndef PLAYER_H
#define	PLAYER_H

#define MAX_STRING_LENGTH 1024
#define MAXIMUM_NUMBER_OF_PLAYERS 10

typedef struct {
    unsigned int id;
    char nickname[MAX_STRING_LENGTH];
    char fifoName[MAX_STRING_LENGTH];
    //TODO card array for hand?
} player_t;

#endif	/* PLAYER_H */

