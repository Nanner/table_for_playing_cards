#include "main.h"

void sigint_handler(int signo) {
    if (isDealer) {
        sem_close(table_ready);
        sem_unlink(semaphoreName);
        
        destroy_table(table, tableName, sizeof (table_t));
        printf("Cleaning dealer variables for shutdown.\n");
        exit(0);
    } else {
        sem_close(table_ready);
        
        if (munmap(table, sizeof (table_t)) < 0) {
            perror("Failure in munmap()");
            exit(EXIT_FAILURE);
        }
        printf("Cleaning player variables for shutdown.\n");
        exit(0);
    }
}

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
        printf("\nPlayer %d, what would you like to do?\n", player.id);
        printf("0- Exit\n1- Check current turn\n2- View cards on table\n");

        int option;
        
        if (currentTurn == player.id) {
            printf("3- Play a card\n");
            option = getChoice("> ", 3);
        }
        else
            option = getChoice("> ", 2);

        switch (option) {
            case 0:
                return NULL;
                break;
            case 1:
                if (currentTurn != player.id)
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
                //printf("Gonna wait for table lock at case 3\n");
                pthread_mutex_lock(&table->tableAccessLock);
                //printf("Gonna wait for syncMut\n");
                pthread_mutex_lock(&syncMut);
                //printf("I have the table access lock at case3 \n");
                //printf("Game interface: Current turn before: %d\n", table->currentTurn);
                table->currentTurn += 1;
                currentTurn++;
                //printf("Current turn up\n");
                //printf("Game interface: Current turn after: %d\n", table->currentTurn);
                if (table->currentTurn >= table->numberOfPlayers) {
                    table->roundNumber++;
                    table->currentTurn = 0;
                }
                copy_cards(table->tableCards, tableCards, DECK_CARDS);
                //printf("Broadcasting\n");
                pthread_cond_broadcast(&table->turnChangeCond);
                //printf("Broadcast done\n");
                pthread_mutex_unlock(&syncMut);
                //printf("Unlocked syncMut\n");
                pthread_mutex_unlock(&table->tableAccessLock);
                //printf("I lost the table access lock at case3 \n");
                
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
        //printf("I have the table access lock at game_sync \n");
        //printf("Locked mutex at game_sync\n");
        while (currentTurn == table->currentTurn) {

            //printf("currentTurn didn't change: %d\n", table->currentTurn);

            //printf("Entering wait\n");
            if (pthread_cond_wait(&table->turnChangeCond, &table->tableAccessLock) != 0)
                printf("What\n");
            //printf("Exiting wait\n");
        }
        pthread_mutex_lock(&syncMut);
        currentTurn = table->currentTurn;
        
        //printf("unlocked mutex at game_sync\n");

        //TODO remove printf crap
        //printf("HOLY CRAP IT WAS BROADCASTED\n");
        
        //printf("Current turn before: %d", currentTurn);
        currentTurn = table->currentTurn;
        if(currentTurn == player.id)
            printf("\n\nIt's your turn!\n\n");
        //printf("Current turn after: %d", currentTurn);
        if (roundNumber < table->roundNumber) {
            printf("\n\nNew round! Current round: %d\n\n", table->roundNumber);
            roundNumber = table->roundNumber;
        }
        copy_cards(tableCards, table->tableCards, DECK_CARDS);
        pthread_mutex_unlock(&syncMut);
        pthread_mutex_unlock(&table->tableAccessLock);
        //printf("I lost the table access lock at game_sync \n");
    }
    return NULL;
}

int main(int argc, char *argv[]) {

    if (argc != 4) {
        printf("Wrong usage: cardTable <player's name> <shm name> <n. players>\n");
        return -1;
    }

    char fifoName[MAX_LEN];
    sprintf(fifoName, "%s_%s", argv[2], argv[1]);
    strcpy(player.nickname, argv[1]);
    strcpy(player.fifoName, fifoName);
    strcpy(tableName, argv[2]);
    playersAwaited = atoi(argv[3]);
    sprintf(semaphoreName, "%s_%s", TABLE_READY_SEM, argv[2]);
    
    /*
    table_ready = sem_open(TABLE_READY_SEM, O_CREAT , 0600, 0);
    sem_close(table_ready);
    sem_unlink(TABLE_READY_SEM);
    destroy_table(table, tableName, sizeof (table_t));
    return 0;
     */

    table_ready = sem_open(semaphoreName, O_CREAT | O_EXCL, 0600, 0);
    if (table_ready == SEM_FAILED) {
        if (errno != EEXIST || (table_ready = sem_open(semaphoreName, O_CREAT, 0600, 0)) == SEM_FAILED) {
            perror("Failure in sem_open()");
            exit(4);
        } else
            printf("You are a player!\n");
    } else {
        isDealer = true;
        printf("You are the dealer!\n");
    }
    
    // install handlers
    // install sigusr1 handler
    struct sigaction action;
    action.sa_handler = sigint_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction(SIGINT, &action, NULL) < 0) {
        fprintf(stderr, "Unable to install SIGINT handler\n");
        exit(7);
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
        
        player.id = 0;
        initialize_local_variables();
        
        printf("adding player...\n");
        
        if( (fifoFD = create_player_fifo(player.fifoName)) == -1) {
            printf("Problem creating the player fifo!\n");
            exit(EXIT_FAILURE);
        }
        
        table->players[table->numberOfPlayers - 1] = player;
        
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
        printf("Gonna wait\n");
        sem_wait(table_ready);
        printf("Waited\n");

        if ((table = attach_table(argv[2], sizeof (table_t))) == NULL) {
            perror("Player: Could not open table");
            exit(EXIT_FAILURE);
        }
        
        //TODO check if player name exists

        player.id = table->numberOfPlayers;
        table->numberOfPlayers++;
        initialize_local_variables();
        
        if( (fifoFD = create_player_fifo(player.fifoName)) == -1) {
            printf("Problem creating the player fifo!\n");
            exit(EXIT_FAILURE);
        }
        
        table->players[table->numberOfPlayers - 1] = player;
        
        printf("Number of players: %d\n", table->numberOfPlayers);
        
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
    
    //TODO make these variables global and put on sigint handler
    close_player_fifo(fifoFD, fifoName);

    if (isDealer) {
        sem_close(table_ready);
        sem_unlink(TABLE_READY_SEM);

        destroy_table(table, tableName, sizeof (table_t));
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