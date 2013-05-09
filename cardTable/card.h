/* 
 * File:   card.h
 * Author: nanner
 *
 * Created on May 9, 2013, 7:22 PM
 */

#ifndef CARD_H
#define	CARD_H

#include <stdbool.h>

#define MAXIMUM_CARDS 52

typedef struct {
    char rank;
    char suit;
} card_t;

card_t usedCard = {.rank = 'n', .suit = 'n'};

bool equal_cards(card_t card1, card_t card2);

card_t* start_deck(card_t* deck);

void shuffle_deck(card_t* deck);

int deck_size(card_t* deck);

bool deck_empty(card_t* deck);

char* get_card_representation(card_t card);

void print_cards(card_t* cards);

#endif	/* CARD_H */

