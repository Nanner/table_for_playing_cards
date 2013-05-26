//  card.h
//
//  SOPE Project 2 - Table for Playing Cards
//
//  Turma 1 Grupo 5
//
//  ei11089 - Diogo Santos
//  ei11122 - Pedro Fernandes
//

#ifndef CARD_H
#define	CARD_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#define DECK_CARDS 52

typedef int card_t;

extern const int usedCard;

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

