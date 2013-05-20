/* 
 * File:   card.h
 * Author: nanner
 *
 * Created on May 9, 2013, 7:22 PM
 */

#ifndef CARD_H
#define	CARD_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#define DECK_CARDS 52

typedef int card_t;

static const card_t usedCard = -1;

static char *ranks[] = { "A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K" };
static const char suits[] = { 'c', 'd', 'h', 's' };

char getSuit(card_t card);

char* getRank(card_t card);

void start_deck(card_t deck[]);

int card_array_size(card_t cards[], int maxSize);

bool deck_empty(card_t deck[]);

void shuffle_deck(card_t deck[]);

void reorder_cards(card_t cards[], int numberOfCards);

char* get_card_representation(card_t card);

char* get_card_array_representation(card_t cards[], int numberOfCards);

char* get_unordered_card_array_representation(card_t cards[], int numberOfCards);

void print_cards(card_t cards[], int numberOfCards);

void print_unordered_cards(card_t cards[], int numberOfCards);

bool give_hand(card_t deck[], card_t hand[], int cardsToGive);

void copy_cards(card_t to[], card_t from[], int numberOfCards);

void initialize_with_usedCard(card_t cards[], int numberOfCards);

#endif	/* CARD_H */

