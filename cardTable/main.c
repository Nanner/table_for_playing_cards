// PROGRAMA p01.c 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <semaphore.h>
#include <sys/types.h>

#include "table.h"

#define TABLE_READY_SEM "/table_ready"

static int isDealer = 0;

static table_t* table;
static sem_t* table_ready;
static pthread_mutex_t syncMut = PTHREAD_MUTEX_INITIALIZER;

static unsigned int currentTurn;
static unsigned int roundNumber;
static card_t tableCards[DECK_CARDS];
//static player_t players[10];

void initialize_local_variables() {
    currentTurn = table->currentTurn;
    roundNumber = table->roundNumber;
    copy_cards(tableCards, table->tableCards, DECK_CARDS);
}

void* game_interface(void* arg) {
    while (currentTurn < 5 && roundNumber < 5) {
        printf("What would you like to do?\n");
        printf("0- Exit\n1- Check current turn\n2- View cards on table\n");
        getchar();
        
        pthread_mutex_lock(&syncMut);
        pthread_mutex_lock(&table->tableAccessLock);
        printf("Game interface: Current turn before: %d\n", table->currentTurn);
        table->currentTurn += 1;
        printf("Current turn up\n");
        printf("Game interface: Current turn after: %d\n", table->currentTurn);
        if (table->currentTurn >= table->numberOfPlayers) {
            table->roundNumber++;
            table->currentTurn = 0;
        }
        copy_cards(table->tableCards, tableCards, DECK_CARDS);
        printf("Broadcasting\n");
        pthread_cond_broadcast(&table->turnChangeCond);
        printf("Broadcast done\n");
        pthread_mutex_unlock(&table->tableAccessLock);
        pthread_mutex_unlock(&syncMut);
    }
    printf("Ready to exit\n");

    return NULL;
}

void* game_sync(void* arg) {
    while (1) {

        pthread_mutex_lock(&table->tableAccessLock);
        printf("Locked mutex at game_sync\n");
        while (currentTurn == table->currentTurn) {

            printf("currentTurn didn't change: %d\n", table->currentTurn);

            printf("Entering wait\n");
            if (pthread_cond_wait(&table->turnChangeCond, &table->tableAccessLock) != 0)
                printf("Wtf\n");
            printf("Exiting wait\n");
        }
        currentTurn = table->currentTurn;
        pthread_mutex_unlock(&table->tableAccessLock);
        printf("unlocked mutex at game_sync\n");

        //TODO remove printf crap
         printf("HOLY CRAP IT WAS BROADCASTED\n");
        
         pthread_mutex_lock(&table->tableAccessLock);
         pthread_mutex_lock(&syncMut);
         printf("Current turn before: %d", currentTurn);
         currentTurn = table->currentTurn;
         printf("Current turn after: %d", currentTurn);
         if (roundNumber < table->roundNumber) {
             printf("New round! Current round: %d", table->roundNumber);
             roundNumber = table->roundNumber;
         }
         copy_cards(tableCards, table->tableCards, DECK_CARDS);
         pthread_mutex_unlock(&syncMut);
         pthread_mutex_unlock(&table->tableAccessLock);
    }
    return NULL;
}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Don't forget the arg\n");
        return -1;
    }

    if (strcmp(argv[1], "s") == 0) {
        isDealer = 1;
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
        
        if(!init_sync_variables_in_table(table)) {
            printf("Failed to initialize the table sync variables!\n");
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

        initialize_local_variables();

        int playerID = table->numberOfPlayers;
        table->numberOfPlayers++;
        initialize_with_usedCard(tableCards, DECK_CARDS);

        sem_post(table_ready);

        printf("Opening threads:\n");
        printf("Gamesync thread\n");

        pthread_t tidSync, tidInterface;
        pthread_create(&tidSync, NULL, game_sync, NULL);

        printf("Interface thread\n");
        pthread_create(&tidInterface, NULL, game_interface, NULL);

        pthread_join(tidInterface, NULL);
        //pthread_join(tidSync, NULL); 


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

        initialize_local_variables();

        int playerID = table->numberOfPlayers;
        table->numberOfPlayers++;
        initialize_with_usedCard(tableCards, DECK_CARDS);

        printf("Opening threads:\n");
        printf("Gamesync thread\n");

        pthread_t tidSync, tidInterface;
        pthread_create(&tidSync, NULL, game_sync, NULL);

        printf("Interface thread\n");
        pthread_create(&tidInterface, NULL, game_interface, NULL);

        /*int playerID = table->numberOfPlayers;
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
        printf("\n\n");*/
        sem_post(table_ready);

        pthread_join(tidInterface, NULL);
        //pthread_join(tidSync, NULL); 

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