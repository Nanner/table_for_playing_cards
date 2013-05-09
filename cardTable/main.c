// PROGRAMA p01.c 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <pthread.h> 
#include <fcntl.h> // For O_* constants 
#include <semaphore.h> 
#include <sys/mman.h> 
#include <sys/mman.h> 
#include <sys/types.h>

#include "table.h"

int main(int argc, char *argv[]) {
    card_t deck[DECK_CARDS];
    printf("Starting deck\n");
    start_deck(deck);
    printf("Deck:\n");
    print_cards(deck, DECK_CARDS);
    printf("\n");
    printf("Shuffling cards\n");
    shuffle_deck(deck);
    printf("Shuffled deck:\n");
    print_cards(deck, DECK_CARDS);
    printf("\n\n");

    int i;
    int cardsToGive = 5;
    for (i = 0; i < 11; i++) {
        printf("Giving %d cards to player %d:\n", cardsToGive, i + 1);
        card_t hand[cardsToGive];
        if (give_hand(deck, hand, cardsToGive)) {
            print_cards(hand, cardsToGive);
            printf("\n");
        } else {
            printf("There's not enough cards on deck to give!\n");
            break;
        }

        printf("%d cards left on deck:\n", deck_size(deck));
        print_cards(deck, DECK_CARDS);
        printf("\n\n");
    }
    getchar();

    return 0;
}