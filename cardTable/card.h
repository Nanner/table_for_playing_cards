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

typedef int card_t; /**< Represents a card with value and suit. */

extern const int usedCard; /**< Determines if a card has been used in the game. */

/**
 * @return Returns the character representing the suit of a card (diamonds, spades, hearts and clubs).
 */
char getSuit(card_t card);

/**
 * @return Returns the string representing the rank of a card (ace, number or figure).
 */
char* getRank(card_t card);

/**
 * Fills an array of cards with used cards.
 * @param cards The array of cards that will be filled.
 * @param numberOfCards The size of the array.
 */
void initialize_with_usedCard(card_t cards[], int numberOfCards);

/**
 * Initializes an array of cards, representing a full deck.
 * @param deck The array that will be initialized.
 */
void start_deck(card_t deck[]);

/**
 * Counts the number of cards in a hand (ignoring the used ones).
 * @param cards The hand that will be counted.
 * @param maxSize The size of the card array.
 * @return The number of valid cards in the array.
 */
int card_array_size(card_t cards[], int maxSize);

/**
 * @param deck The array of cards.
 * @return True if a deck has no valid cards, false otherwise.
 */
bool deck_empty(card_t deck[]);

/**
 * Shuffles the order of the cards in a deck.
 * @param deck The deck that will be shuffled.
 */
void shuffle_deck(card_t deck[]);

/**
 * Moves all used cards from their position in the array to the end of the array of cards.
 * @param cards The array of cards.
 * @param numberOfCards The size of the array.
 */
void reorder_used_cards(card_t cards[], int numberOfCards);

/**
 * @param card The card that will be represented.
 * @return A string with the representation of a given card (rank and suit).
 */
char* get_card_representation(card_t card);

/**
 * This function will display an array of cards in a sorted manner.
 * @param cards The card array that will be represented.
 * @param numberOfCards The size of the card array.
 * @return A string with the representation of all the cards in the array (ordered by ranks and suits).
 */
char* get_card_array_representation(card_t cards[], int numberOfCards);

/**
 * This function will display a array of cards.
 * @param cards The card array that will be represented.
 * @param numberOfCards The size of the card array.
 * @return A string with the representation of all the cards in the array.
 */
char* get_unordered_card_array_representation(card_t cards[], int numberOfCards);

/**
 * Prints an array of cards in a sorted manner.
 * @param cards The array of cards.
 * @param numberOfCards Size of the array of cards.
 */
void print_cards(card_t cards[], int numberOfCards);

/**
 * Prints an array of cards.
 * @param cards The array of cards.
 * @param numberOfCards Size of the array of cards.
 */
void print_unordered_cards(card_t cards[], int numberOfCards);

/**
 * Removes a number of cards from the deck and puts them into the specified hand of cards.
 * @param deck The deck that will give the cards.
 * @param hand The hand that will receive the cards.
 * @param cardsToGive The number of cards to give.
 * @return True if the hand received the number of specified cards successfully, false otherwise.
 */
bool give_hand(card_t deck[], card_t hand[], int cardsToGive);

/**
 * Copies a card array to another array.
 * @param to The card array that will be copied.
 * @param from The new card array.
 * @param numberOfCards The size of the card array that will be copied.
 */
void copy_cards(card_t to[], card_t from[], int numberOfCards);

#endif	/* CARD_H */

