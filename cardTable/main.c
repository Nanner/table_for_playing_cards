// PROGRAMA p01.c 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <semaphore.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>

#include "table.h"

#define TABLE_READY_SEM "/table_ready2"
#define MAX_LEN 1024

static int playerID;
static player_t player;
static bool isDealer = false;

static table_t* table;
static sem_t* table_ready;
static pthread_mutex_t syncMut = PTHREAD_MUTEX_INITIALIZER;
static int fifoFD;

static unsigned int playersAwaited;
static unsigned int currentTurn;
static unsigned int roundNumber;
static card_t tableCards[DECK_CARDS];
static card_t handCards[HAND_CARDS];
//static player_t players[10];

void initialize_local_variables() {
    currentTurn = table->currentTurn;
    roundNumber = table->roundNumber;
    copy_cards(tableCards, table->tableCards, DECK_CARDS);
}

int getChoice(const char *prompt, int maxChoice) {
    int choice;
    char input[MAX_LEN];

    while (fputs(prompt, stdout) != EOF && fgets(input, sizeof (input), stdin) != 0) {
        if (sscanf(input, "%d", &choice) == 1) {
            if (choice >= 0 && choice <= maxChoice) {
                return choice;
            } else {
                printf("Please select an available option\n");
            }
        }
        printf("That's not a valid input!\n");
    }

    printf("EOF problem or error!\n");
    exit(1);
}

void* game_interface(void* arg) {
    while (1) {
        printf("Your hand:\n");
        print_cards(handCards, HAND_CARDS);
        printf("\nPlayer %d, what would you like to do?\n", playerID);
        printf("0- Exit\n1- Check current turn\n2- View cards on table\n");

        if (currentTurn == playerID)
            printf("3- Play a card\n");

        int option = getChoice("> ", 3);

        switch (option) {
            case 0:
                return NULL;
                break;
            case 1:
                if (currentTurn != playerID)
                    printf("It's player %d's turn\n", currentTurn);
                else
                    printf("It's your turn!\n");
                break;
            case 2:
                if (deck_size(tableCards) != 0) {
                    printf("Cards on table:\n");
                    print_cards(tableCards, DECK_CARDS);
                } else
                    printf("There are no cards on the table!\n");
                break;
            case 3:
                printf("Gonna wait for table lock at case 3\n");
                pthread_mutex_lock(&table->tableAccessLock);
                printf("Gonna wait for syncMut\n");
                pthread_mutex_lock(&syncMut);
                printf("I have the table access lock at case3 \n");
                printf("Game interface: Current turn before: %d\n", table->currentTurn);
                table->currentTurn += 1;
                currentTurn++;
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
                pthread_mutex_unlock(&syncMut);
                printf("Unlocked syncMut\n");
                pthread_mutex_unlock(&table->tableAccessLock);
                printf("I lost the table access lock at case3 \n");
                
                break;
            default:
                printf("Invalid option!\n");
                break;
        }
    }
    printf("Ready to exit\n");

    return NULL;
}

void* game_sync(void* arg) {
    while (1) {

        pthread_mutex_lock(&table->tableAccessLock);
        printf("I have the table access lock at game_sync \n");
        printf("Locked mutex at game_sync\n");
        while (currentTurn == table->currentTurn) {

            printf("currentTurn didn't change: %d\n", table->currentTurn);

            printf("Entering wait\n");
            if (pthread_cond_wait(&table->turnChangeCond, &table->tableAccessLock) != 0)
                printf("Wtf\n");
            printf("Exiting wait\n");
        }
        pthread_mutex_lock(&syncMut);
        currentTurn = table->currentTurn;
        
        printf("unlocked mutex at game_sync\n");

        //TODO remove printf crap
        printf("HOLY CRAP IT WAS BROADCASTED\n");
        
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
        printf("I lost the table access lock at game_sync \n");
    }
    return NULL;
}

int main(int argc, char *argv[]) {

    if (argc != 4) {
        printf("Wrong usage: cardTable <player's name> <shm name> <n. players>\n");
        return -1;
    }

    strcpy(player.nickname, argv[1]);
    strcpy(player.fifoName, argv[1]);
    playersAwaited = atoi(argv[3]);

    table_ready = sem_open(TABLE_READY_SEM, O_CREAT | O_EXCL, 0600, 0);
    if (table_ready == SEM_FAILED) {
        if (errno != EEXIST || (table_ready = sem_open(TABLE_READY_SEM, O_CREAT, 0600, 0)) == SEM_FAILED) {
            perror("Failure in sem_open()");
            exit(4);
        } else
            printf("You are a player!\n");
    } else {
        isDealer = true;
        printf("You are the dealer!\n");
    }

    if (isDealer) {
        
        if ((table = create_table(argv[2], sizeof (table_t))) == NULL) {
            perror("Dealer: Could not create table");
            exit(EXIT_FAILURE);
        }

        if (!init_sync_variables_in_table(table)) {
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
        
        playerID = 0;
        player.id = playerID;
        initialize_local_variables();
        
        printf("adding player...\n");
        
        //table->players[table->numberOfPlayers - 1] = player;
        
        printf("Toggling semaphore\n");

        if(sem_post(table_ready) == -1) {
            perror("Problem unlocking table_ready semaphore");
            exit(EXIT_FAILURE);
        }
        
        printf("Waiting for %d players...\n", playersAwaited);
        pthread_mutex_lock(&table->playerWaitLock);
        while (table->numberOfPlayers < playersAwaited)
            pthread_cond_wait(&table->playerWaitCond, &table->playerWaitLock);
        //deal_cards();
        pthread_mutex_unlock(&table->playerWaitLock);

    } else {
        sem_wait(table_ready);

        if ((table = attach_table(argv[2], sizeof (table_t))) == NULL) {
            perror("Player: Could not open table");
            exit(EXIT_FAILURE);
        }

        playerID = table->numberOfPlayers;
        table->numberOfPlayers++;
        player.id = playerID;
        initialize_local_variables();
        
        if( (fifoFD = create_player_fifo(player.fifoName)) == -1) {
            printf("Problem creating the player fifo!\n");
            exit(EXIT_FAILURE);
        }
        
        table->players[table->numberOfPlayers - 1] = player;
        
        if(table->numberOfPlayers == playersAwaited)
            pthread_cond_broadcast(&table->playerWaitCond);
        
        sem_post(table_ready);
        
        printf("Waiting for %d players...", playersAwaited);
        pthread_mutex_lock(&table->playerWaitLock);
        while (table->numberOfPlayers < playersAwaited)
            pthread_cond_wait(&table->playerWaitCond, &table->playerWaitLock);
        pthread_mutex_unlock(&table->playerWaitLock);
           
    }

    initialize_with_usedCard(tableCards, DECK_CARDS);

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

    if (isDealer) {
        sem_close(table_ready);
        sem_unlink(TABLE_READY_SEM);

        destroy_table(table, argv[2], sizeof (table_t));
    } else {
        sem_close(table_ready);

        if (munmap(table, sizeof (table_t)) < 0) {
            perror("Failure in munmap()");
            exit(EXIT_FAILURE);
        }
    }

    exit(EXIT_SUCCESS);

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
}