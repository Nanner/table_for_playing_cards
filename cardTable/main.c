#include "main.h"

void sigint_handler(int signo) {
    close_player_fifo(fifoFD, fifoName);
    
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

void deal_cards() {
    
    int i;
    for(i = 0; i < table->numberOfPlayers; i++) {
        card_t hand[HAND_CARDS];
        give_hand(table->deck, hand, HAND_CARDS);
        printf("Giving hand:\n");
        print_cards(hand, HAND_CARDS);
        printf("\n");
        send_hand_to_fifo(table->players[i].fifoName, hand, HAND_CARDS);
    }
    
}

void play_card() {
    printf("Your cards:\n");
    print_cards(handCards, card_array_size(handCards, HAND_CARDS));
    printf("\n");
    
    char prompt[MAX_LEN];
    sprintf(prompt, "Choose a card to play: (1 to %d, 0 to go back)\n> ", card_array_size(handCards, HAND_CARDS));
    int choice = getChoice(prompt, HAND_CARDS);
    
    if(choice == 0)
        return;
    
    pthread_mutex_lock(&table->tableAccessLock);
    pthread_mutex_lock(&syncMut);
    
    printf("Table cards before:\n");
    print_cards(tableCards, numberOfCardsOnTable);
    printf("\n");
    
    //TODO fix whatever is wrong here
    
    printf("Playing card %s\n", get_card_representation(handCards[choice-1]));
    tableCards[table->numberOfCardsOnTable] = handCards[choice-1];
    table->numberOfCardsOnTable++;
    handCards[choice-1] = usedCard;
    reorder_cards(handCards, HAND_CARDS);
    
    printf("Table cards after:\n");
    print_cards(tableCards, numberOfCardsOnTable);
    printf("\n");
    
    
    currentTurn++;
    table->currentTurn += 1;
    
    int lastTurn;
    
    if(table->firstPlayerID != 0)
        lastTurn = table->firstPlayerID;
    else
        lastTurn = table->numberOfPlayers;
    
    if (table->currentTurn == lastTurn) {
        if(card_array_size(handCards, HAND_CARDS) == 0) {
            table->gameState = STOP_GAME;
        }
        table->roundNumber++;
        table->currentTurn = table->firstPlayerID;
    }
    
    if(table->currentTurn == table->numberOfPlayers) {
        currentTurn = 0;
        table->currentTurn = 0;
    }
    
    copy_cards(table->tableCards, tableCards, DECK_CARDS);
  
    pthread_cond_broadcast(&table->turnChangeCond);
    pthread_mutex_unlock(&syncMut);
    pthread_mutex_unlock(&table->tableAccessLock);
}

void initialize_local_variables() {
    currentTurn = table->currentTurn;
    roundNumber = table->roundNumber;
    copy_cards(tableCards, table->tableCards, DECK_CARDS);
    numberOfCardsOnTable = table->numberOfCardsOnTable;
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
        print_cards(handCards, card_array_size(handCards, HAND_CARDS));
        printf("\nPlayer %d, what would you like to do?\n", player.id);
        printf("0- Exit\n1- Update\n2- Check current turn\n3- View cards on table\n");

        int option;
        
        if (currentTurn == player.id) {
            printf("4- Play a card\n");
            option = getChoice("> ", 4);
        }
        else
            option = getChoice("> ", 3);

        switch (option) {
            case 0:
                pthread_exit(NULL);
                break;
            case 1:
                break;
            case 2:
                if (currentTurn != player.id)
                    printf("It's player %d's turn\n", currentTurn);
                else
                    printf("It's your turn!\n");
                break;
            case 3:
                if (card_array_size(tableCards, DECK_CARDS) != 0) {
                    printf("Cards on table:\n");
                    print_cards(tableCards, numberOfCardsOnTable);
                    printf("\n");
                } else
                    printf("There are no cards on the table!\n");
                break;
            case 4:
                play_card();
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
        while (currentTurn == table->currentTurn) {
            if (pthread_cond_wait(&table->turnChangeCond, &table->tableAccessLock) != 0)
                printf("Failed on cond wait for turnChangeCond!\n");
        }
        pthread_mutex_unlock(&table->tableAccessLock);
        
        pthread_mutex_lock(&table->tableAccessLock);
        printf("Game state: %d\n", table->gameState);
        if(table->gameState == STOP_GAME){
                printf("No more cards left to play, game over!\n");
                pthread_mutex_unlock(&table->tableAccessLock);
                pthread_cancel(tidInterface);
                pthread_exit(NULL);
        }
        
        pthread_mutex_lock(&syncMut);
        copy_cards(tableCards, table->tableCards, DECK_CARDS);
        numberOfCardsOnTable = table->numberOfCardsOnTable;
        
        char* cardPlayed = get_card_representation(tableCards[table->numberOfCardsOnTable - 1]);
        printf("Player %d has played %s\n", currentTurn, cardPlayed);
        free(cardPlayed);
        
        currentTurn = table->currentTurn;
        if(currentTurn == player.id) {
            printf("\n\nIt's your turn!\nPress 1 to update!\n\n");
        }
        if (roundNumber < table->roundNumber) {
            printf("\n\nNew round! Current round: %d\nPress 1 to update!\n\n", table->roundNumber);
            roundNumber = table->roundNumber;
        }
   
        pthread_mutex_unlock(&syncMut);
        pthread_mutex_unlock(&table->tableAccessLock);
    }
    return NULL;
}

int main(int argc, char *argv[]) {

    if (argc != 4) {
        printf("Wrong usage: cardTable <player's name> <shm name> <n. players>\n");
        return -1;
    }
    
    //TODO check if names already exist

    sprintf(fifoName, "%s_%s", argv[2], argv[1]);
    strcpy(player.nickname, argv[1]);
    strcpy(player.fifoName, fifoName);
    strcpy(tableName, argv[2]);
    playersAwaited = atoi(argv[3]);
    sprintf(semaphoreName, "%s_%s", TABLE_READY_SEM, argv[2]);
    
    /*
    table_ready = sem_open(semaphoreName, O_CREAT , 0600, 0);
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

        start_deck(table->deck);
        shuffle_deck(table->deck);
        
        player.id = 0;
        
        if( (fifoFD = create_player_fifo(player.fifoName)) == -1) {
            printf("Problem creating the player fifo!\n");
            exit(EXIT_FAILURE);
        }
        
        table->players[table->numberOfPlayers - 1] = player;

        if(sem_post(table_ready) == -1) {
            perror("Problem unlocking table_ready semaphore");
            exit(EXIT_FAILURE);
        }
        
        printf("Waiting for %d players...\n", playersAwaited);
        pthread_mutex_lock(&table->playerWaitLock);
        while (table->numberOfPlayers < playersAwaited)
            pthread_cond_wait(&table->playerWaitCond, &table->playerWaitLock);
        deal_cards();
        table->cardsDealt = 1;
        srand(time(NULL));
        table->firstPlayerID = rand() % table->numberOfPlayers;
        table->currentTurn = table->firstPlayerID;
        initialize_local_variables();
        printf("First one to play is player %d!\n", table->firstPlayerID);
        pthread_cond_broadcast(&table->dealingCardsCond);
        pthread_mutex_unlock(&table->playerWaitLock);
        
        get_hand_from_fifo(fifoFD, handCards, HAND_CARDS);

    } else {
        sem_wait(table_ready);

        if ((table = attach_table(argv[2], sizeof (table_t))) == NULL) {
            perror("Player: Could not open table");
            exit(EXIT_FAILURE);
        }
        
        //TODO check if player name exists

        player.id = table->numberOfPlayers;
        table->numberOfPlayers++;
        
        if( (fifoFD = create_player_fifo(player.fifoName)) == -1) {
            printf("Problem creating the player fifo!\n");
            exit(EXIT_FAILURE);
        }
        
        table->players[table->numberOfPlayers - 1] = player;
        
        printf("Number of players on table: %d\n", table->numberOfPlayers);
        
        if(table->numberOfPlayers == playersAwaited)
            pthread_cond_signal(&table->playerWaitCond);
        
        sem_post(table_ready);
        
        printf("Waiting for %d players...\n", playersAwaited);
        pthread_mutex_lock(&table->dealingCardsLock);
        while (table->cardsDealt != 1)
            pthread_cond_wait(&table->dealingCardsCond, &table->dealingCardsLock);
        printf("First one to play is player %d!\n", table->firstPlayerID);
        pthread_mutex_unlock(&table->dealingCardsLock);
        
        initialize_local_variables();
        
        get_hand_from_fifo(fifoFD, handCards, HAND_CARDS);
           
    }
    
    initialize_with_usedCard(tableCards, DECK_CARDS);

    pthread_create(&tidSync, NULL, game_sync, NULL);
    pthread_create(&tidInterface, NULL, game_interface, NULL);

    pthread_join(tidInterface, NULL);
    pthread_cancel(tidSync);


    printf("Press enter to leave\n");
    getchar();
    
    close_player_fifo(fifoFD, fifoName);

    if (isDealer) {
        sem_close(table_ready);
        sem_unlink(semaphoreName);

        destroy_table(table, tableName, sizeof (table_t));
    } else {
        sem_close(table_ready);

        if (munmap(table, sizeof (table_t)) < 0) {
            perror("Failure in munmap()");
            exit(EXIT_FAILURE);
        }
    }

    exit(EXIT_SUCCESS);
}