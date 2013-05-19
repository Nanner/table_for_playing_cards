//
//  main.h
//  cardtable
//
//  Created by pedro on 5/15/13.
//  Copyright (c) 2013 feup. All rights reserved.
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

static int gameState = CONTINUE_GAME;

static player_t player;
static bool isDealer = false;

static char semaphoreName[MAX_LEN];
static char tableName[MAX_LEN];
static table_t* table;
static sem_t* table_ready;
static pthread_mutex_t syncMut = PTHREAD_MUTEX_INITIALIZER;
static int fifoFD;
static char fifoName[MAX_LEN];
static pthread_t tidSync, tidInterface;

static unsigned int playersAwaited;
static unsigned int currentTurn;
static unsigned int roundNumber;

static card_t tableCards[DECK_CARDS];
static unsigned int numberOfCardsOnTable;

static card_t handCards[HAND_CARDS];


#endif
