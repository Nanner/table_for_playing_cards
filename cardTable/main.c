// PROGRAMA p01.c 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>

#include "table.h"

static table_t* table;
static sem_t* table_ready;

#define TABLE_READY_SEM "table_ready"

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Don't forget the arg\n");
        return -1;
    }

    if (strcmp(argv[1], "s") == 0) {

        printf("Starting server (dealer)\n");

        table_ready = sem_open(TABLE_READY_SEM, O_CREAT, 0600, 0);
        if (table_ready == SEM_FAILED) {
            perror("FILLER failure in sem_open()");
            exit(4);
        }

        if ((table = create_table("table1", sizeof (table_t))) == NULL) {
            perror("Dealer: Could not create table");
            exit(EXIT_FAILURE);
        }

        printf("Starting deck\n");
        start_deck(table->deck);
        printf("Deck:\n");
        print_cards(table->deck, DECK_CARDS);
        printf("\n");
        printf("Shuffling cards\n");
        shuffle_deck(table->deck);
        printf("Shuffled deck:\n");
        print_cards(table->deck, DECK_CARDS);
        printf("\n\n");

        sem_post(table_ready);

        printf("Press enter to leave\n");
        getchar();

        sem_close(table_ready);
        sem_unlink(TABLE_READY_SEM);

        destroy_table(table, "table1", sizeof (table_t));

    } else if (strcmp(argv[1], "c") == 0) {
        printf("Starting client (normal player)\n");

        //open existing semaphores
        table_ready = sem_open(TABLE_READY_SEM, 0, 0600, 0);
        if (table_ready == SEM_FAILED) {
            perror("Player failure in sem_open() for table_ready");
            exit(3);
        }

        sem_wait(table_ready);

        if ((table = attach_table("table1", sizeof (table_t))) == NULL) {
            perror("Player: Could not open table");
            exit(EXIT_FAILURE);
        }

        int playerID = table->numberOfPlayers;
        table->numberOfPlayers++;

        int cardsToGive = 5;
        printf("Giving %d cards to player %d:\n", cardsToGive, playerID);
        card_t hand[cardsToGive];
        if (give_hand(table->deck, hand, cardsToGive)) {
            print_cards(hand, cardsToGive);
            printf("\n");
        } else {
            printf("There's not enough cards on deck to give!\n");
        }

        printf("%d cards left on deck:\n", deck_size(table->deck));
        print_cards(table->deck, DECK_CARDS);
        printf("\n\n");
        sem_post(table_ready);

        printf("Press enter to quit\n");
        getchar();

        sem_close(table_ready);

        if (munmap(table, sizeof (table_t)) < 0) {
            perror("Failure in munmap()");
            exit(EXIT_FAILURE);
        }

        exit(EXIT_SUCCESS);

    }

    return 0;
}