#include "main.h"
#include "event.h"

//TODO add missing functions to header

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

void add_event(char* eventType, card_t result[], int numberOfCardsInResult) {
    event_t event;
    strcpy(event.eventType, eventType);
    event.playerID = player.id;
    strcpy(event.playerName, player.nickname);
    time(&event.timeStamp);
    
    event.numberOfCardsInResult = numberOfCardsInResult;
    
    int i;
    for(i = 0; i < numberOfCardsInResult; i++)
        event.result[i] = result[i];
    
    pthread_mutex_lock(&table->addingEventLock);
    table->eventList[table->numberOfEvents] = event;
    table->numberOfEvents++;
    write_to_log(tableName, event);
    pthread_mutex_unlock(&table->addingEventLock);
}

void add_event_without_result(char* eventType) {
    event_t event;
    strcpy(event.eventType, eventType);
    event.playerID = player.id;
    strcpy(event.playerName, player.nickname);
    time(&event.timeStamp);
    
    event.numberOfCardsInResult = 0;
    
    pthread_mutex_lock(&table->addingEventLock);
    table->eventList[table->numberOfEvents] = event;
    table->numberOfEvents++;
    write_to_log(tableName, event);
    pthread_mutex_unlock(&table->addingEventLock);
}

void deal_cards() {
    
    int i;
    for(i = 0; i < table->numberOfPlayers; i++) {
        card_t hand[HAND_CARDS];
        give_hand(table->deck, hand, HAND_CARDS);
        send_hand_to_fifo(table->players[i].fifoName, hand, HAND_CARDS);
    }
    
}

void check_turn_elapsed_time() {
    time_t currentTime = time(NULL);
    pthread_mutex_lock(&table->tableAccessLock);
    double seconds = difftime(currentTime, table->turnStartTimestamp);
    printf("Player %d's turn\n", table->currentTurn);
    pthread_mutex_unlock(&table->tableAccessLock);
    printf("%.2g seconds elapsed since turn start\n", seconds);
}

void check_round_elapsed_time() {
    time_t currentTime = time(NULL);
    pthread_mutex_lock(&table->tableAccessLock);
    double seconds = difftime(currentTime, table->roundStartTimestamp);
    printf("Round %d\n", table->roundNumber);
    pthread_mutex_unlock(&table->tableAccessLock);
    printf("%.2g seconds elapsed since round start\n", seconds);
}


void play_card() {
    
    // Show player's cards
    printf("Your cards:\n");
    print_cards(handCards, card_array_size(handCards, HAND_CARDS));
    printf("\n");
    
    // Ask for the card to play, or give option to go back
    char prompt[MAX_LEN];
    sprintf(prompt, "Choose a card to play: (1 to %d, 0 to go back)\n> ", card_array_size(handCards, HAND_CARDS));
    int choice = getChoice(prompt, HAND_CARDS);
    
    if(choice == 0)
        return;
    
    // Show the card being played
    printf("Playing card %s\n", get_card_representation(handCards[choice-1]));
    
    // Lock table access mutex and local mutex for synch purposes
    pthread_mutex_lock(&table->tableAccessLock);
    pthread_mutex_lock(&syncMut);
    
    // Updating the table cards on the shared memory and number of cards on table
    table->tableCards[table->numberOfCardsOnTable] = handCards[choice-1];
    table->numberOfCardsOnTable++;
    
    card_t result[1];
    result[0] = handCards[choice-1];
    add_event("play", result , 1);
    add_event("table", table->tableCards , table->numberOfCardsOnTable);
    
    // Removing the played card from the hand cards and reordering it for array access purposes
    handCards[choice-1] = usedCard;
    reorder_cards(handCards, HAND_CARDS);
    add_event("hand", handCards , card_array_size(handCards, HAND_CARDS));
   
    // Updating current turn
    currentTurn++;
    table->currentTurn += 1;
    
    // Check if there's a new round (all players already played this round)
    int lastTurn;
    
    if(table->firstPlayerID != 0)
        lastTurn = table->firstPlayerID;
    else
        lastTurn = table->numberOfPlayers;
    
    if (table->currentTurn == lastTurn) {
        if(card_array_size(handCards, HAND_CARDS) == 0) {
            table->gameState = STOP_GAME;
            gameState = STOP_GAME;
            printf("No more cards left to play, game over!\n");
            add_event_without_result("gameover");
        }
        table->roundNumber++;
        // Update round timestamp
        time(&table->roundStartTimestamp);
        table->currentTurn = table->firstPlayerID;
    }
    
    // Check if we need to "loop" currentTurn back to 0
    if(table->currentTurn == table->numberOfPlayers) {
        currentTurn = 0;
        table->currentTurn = 0;
    }
    
    // Update local table cards
    copy_cards(tableCards, table->tableCards, DECK_CARDS);
    numberOfCardsOnTable++;
    
    // Update turn timestamp
    time(&table->turnStartTimestamp);
  
    // Broadcast the turn change and unlock mutexes
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
    pthread_mutex_lock(&syncMut);
    unsigned int synchedGameState = gameState;
    pthread_mutex_unlock(&syncMut);
    while (synchedGameState == CONTINUE_GAME) {
        
        if(card_array_size(handCards, HAND_CARDS) > 0) {
                printf("Your hand:\n");
                print_cards(handCards, card_array_size(handCards, HAND_CARDS));
                printf("\n\n");
        }
        else
            printf("\nYou have no more cards to play!\n\n");
        
        printf("Player %d, what would you like to do?\n", player.id);
        printf("0- Show table history\n1- Update\n2- Check current turn\n3- View cards on table\n4- Check current turn elapsed time"
                "\n5- Check current round elapsed time\n");

        int option;
        
        if (currentTurn == player.id) {
            printf("6- Play a card\n");
            option = getChoice("> ", 6);
        }
        else
            option = getChoice("> ", 5);

        switch (option) {
            case 0:
                pthread_mutex_lock(&table->tableAccessLock);
                print_event_list_for_player(table->eventList, table->numberOfEvents);
                pthread_mutex_unlock(&table->tableAccessLock);
                
                printf("Press enter to continue.\n");
                getchar();
                break;
            case 1:
                break;
            case 2:
                if (currentTurn != player.id)
                    printf("\nIt's player %d's turn\n", currentTurn);
                else
                    printf("\nIt's your turn!\n");
                
                printf("Press enter to continue.\n");
                getchar();
                break;
            case 3:
                if (card_array_size(tableCards, DECK_CARDS) != 0) {
                    printf("Cards on table:\n");
                    print_unordered_cards(tableCards, numberOfCardsOnTable);
                    printf("\n");
                } else
                    printf("There are no cards on the table!\n");
                
                printf("Press enter to continue.\n");
                getchar();
                break;
            case 4:
                check_turn_elapsed_time();
                
                printf("Press enter to continue.\n");
                getchar();
                break;
            case 5:
                check_round_elapsed_time();
                
                printf("Press enter to continue.\n");
                getchar();
                break;
            case 6:
                play_card();
                break;
            default:
                printf("Invalid option!\n");
                break;
        }
        
        pthread_mutex_lock(&syncMut);
        synchedGameState = gameState;
        pthread_mutex_unlock(&syncMut);
    
    }
    printf("Ready to exit\n");

    return NULL;
}

void* game_sync(void* arg) {
    pthread_mutex_lock(&syncMut);
    unsigned int synchedGameState = gameState;
    pthread_mutex_unlock(&syncMut);
    while (synchedGameState == CONTINUE_GAME) {
        /* Wait for a turn change before update */
        pthread_mutex_lock(&table->tableAccessLock);
        while (currentTurn == table->currentTurn) {
            if (pthread_cond_wait(&table->turnChangeCond, &table->tableAccessLock) != 0){
                printf("Failed on cond wait for turnChangeCond!\n");
                // TODO getting EINVAL here, fix please
                sigint_handler(SIGINT);
                exit(1);
            }
        }
        pthread_mutex_unlock(&table->tableAccessLock);
        
        /* Updating, locking the table to prevent synch problems */
        pthread_mutex_lock(&table->tableAccessLock);
        
        // Updating local variables, locking the local mutex
        pthread_mutex_lock(&syncMut);
        
        // Update local game state
        gameState = table->gameState;
        
        // Getting table cards and number of cards on table
        copy_cards(tableCards, table->tableCards, DECK_CARDS);
        numberOfCardsOnTable = table->numberOfCardsOnTable;
        
        // Informing the player of the card played in the previous round
        char* cardPlayed = get_card_representation(tableCards[table->numberOfCardsOnTable - 1]);
        printf("\n\nPlayer %d has played %s\n\n", currentTurn, cardPlayed);
        free(cardPlayed);
        
        // Update current turn, informing the player if it's his turn or if there was a round change
        currentTurn = table->currentTurn;
        if(currentTurn == player.id) {
            printf("\n\nIt's your turn!\n");
        }
        
        if (roundNumber < table->roundNumber) {
            printf("\nNew round! Current round: %d\n", table->roundNumber);
            roundNumber = table->roundNumber;
        }
        
        printf("\nPress 1 to update!\n\n");
   
        synchedGameState = gameState;
        pthread_mutex_unlock(&syncMut);
        pthread_mutex_unlock(&table->tableAccessLock);
    }
    
    printf("No more cards left to play, game over!\n");
    pthread_cancel(tidInterface);
    return NULL;
}

int main(int argc, char *argv[]) {
    
    /*event_t testEvent;
    strcpy(testEvent.eventType, "deal");
    testEvent.numberOfCardsInResult = 0;
    testEvent.playerID = 0;
    strcpy(testEvent.playerName, "Manuel");
    //testEvent.result = NULL;
    testEvent.timeStamp = time(NULL);
    
    print_event(testEvent);
    return;*/
    
    /*
    card_t baralhoFixe[DECK_CARDS];
    start_deck(baralhoFixe);
    print_cards(baralhoFixe, DECK_CARDS);
    return;
    */

    if (argc != 4) {
        printf("Wrong usage: cardTable <player's name> <shm name> <n. players>\n");
        return -1;
    }
    
    //TODO check if names already exist

    // Get necessary variables from the arguments
    sprintf(fifoName, "%s_%s", argv[2], argv[1]);
    strcpy(player.nickname, argv[1]);
    strcpy(player.fifoName, fifoName);
    strcpy(tableName, argv[2]);
    playersAwaited = atoi(argv[3]);
    sprintf(semaphoreName, "%s_%s", TABLE_READY_SEM, argv[2]);
    
    /*
     // to clean up when we program crashes
    table_ready = sem_open(semaphoreName, O_CREAT , 0600, 0);
    sem_close(table_ready);
    sem_unlink(TABLE_READY_SEM);
    destroy_table(table, tableName, sizeof (table_t));
    return 0;
     */

    // Try to open the table_ready semaphore. If it doesn't exist, you are the dealer, if it does, you're a player
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
    
    // Memory and FIFO initializations
    if (isDealer) {
        
        // Create shared memory (table)
        if ((table = create_table(argv[2], sizeof (table_t))) == NULL) {
            perror("Dealer: Could not create table");
            exit(EXIT_FAILURE);
        }

        if (!init_sync_variables_in_table(table)) {
            printf("Failed to initialize the table sync variables!\n");
            exit(EXIT_FAILURE);
        }
        
        // Indicate the number of players that should play in this table
        table->playersAwaited = playersAwaited;

        // Create a deck and shuffle it
        start_deck(table->deck);
        shuffle_deck(table->deck);
        
        // Give the player id)
        player.id = 0;
        
        // Create the player FIFO
        if( (fifoFD = create_player_fifo(player.fifoName)) == -1) {
            printf("Problem creating the player fifo!\n");
            exit(EXIT_FAILURE);
        }
        
        // Add player to player array on shared memory
        table->players[table->numberOfPlayers - 1] = player;

        // Unlock table_ready semaphore
        if(sem_post(table_ready) == -1) {
            perror("Problem unlocking table_ready semaphore");
            exit(EXIT_FAILURE);
        }
        
        // Wait for the correct number of players to show
        printf("Waiting for %d players...\n", playersAwaited);
        pthread_mutex_lock(&table->playerWaitLock);
        while (table->numberOfPlayers < playersAwaited)
            pthread_cond_wait(&table->playerWaitCond, &table->playerWaitLock);
        
        initialize_event_log(tableName);
        
        // After all players are in game, deal cards and decide the first player to play
        deal_cards();
        table->cardsDealt = 1;
        add_event_without_result("deal");
        srand(time(NULL));
        table->firstPlayerID = rand() % table->numberOfPlayers;
        table->currentTurn = table->firstPlayerID;
        
        initialize_local_variables();
        
        printf("First one to play is player %d!\n", table->firstPlayerID);
        
        // Get dealt hand from the player FIFO
        get_hand_from_fifo(fifoFD, handCards, HAND_CARDS);
        add_event("receive_cards", handCards , HAND_CARDS);
        
        // Start turn and round timestamp
        time(&table->turnStartTimestamp);
        time(&table->roundStartTimestamp);
        
        // Broadcast that the dealing of cards is done
        pthread_cond_broadcast(&table->dealingCardsCond);
        pthread_mutex_unlock(&table->playerWaitLock);

    } else {
        sem_wait(table_ready);

        // Open the shared memory (table)
        if ((table = attach_table(argv[2], sizeof (table_t))) == NULL) {
            perror("Player: Could not open table");
            sem_post(table_ready);
            exit(EXIT_FAILURE);
        }
        
        // Indicate number of players already on table. If the number of awaited players is already achieved, cancel player entrance.
        printf("Number of players on table: %d\n", table->numberOfPlayers);
        if (table->numberOfPlayers == table->playersAwaited) {
            
            printf("This table is already full, please join a different one!\n");
            
            // Post and close the semaphore and the shared memory and exit
            sem_post(table_ready);
            sem_close(table_ready);

            if (munmap(table, sizeof (table_t)) < 0) {
                perror("Failure in munmap()");
                exit(EXIT_FAILURE);
            }
            
            exit(EXIT_SUCCESS);
        }
        
        // Check if player name exists
        if (check_if_player_exists(table, player.nickname)) {
            
            printf("A player with this name already exists, please pick a different one!\n");
            
            // Post and close the semaphore and the shared memory and exit
            sem_post(table_ready);
            sem_close(table_ready);

            if (munmap(table, sizeof (table_t)) < 0) {
                perror("Failure in munmap()");
                exit(EXIT_FAILURE);
            }
            
            exit(EXIT_SUCCESS);
            
        }

        // Get player id and increment number of players
        player.id = table->numberOfPlayers;
        table->numberOfPlayers++;
        
        // Create the player's FIFO
        if( (fifoFD = create_player_fifo(player.fifoName)) == -1) {
            printf("Problem creating the player fifo!\n");
            
            // Post and close the semaphore and the shared memory and exit
            sem_post(table_ready);
            sem_close(table_ready);

            if (munmap(table, sizeof (table_t)) < 0) {
                perror("Failure in munmap()");
                exit(EXIT_FAILURE);
            }
            
            exit(EXIT_FAILURE);
        }
        
        // Add player to the player array on table
        table->players[table->numberOfPlayers - 1] = player;
        
        // If the table has reached the awaited player number, signal so the dealer can deal cards and take care of the rest
        if(table->numberOfPlayers == table->playersAwaited)
            pthread_cond_signal(&table->playerWaitCond);
        
        sem_post(table_ready);
        
        // Wait for the number of awaited players to be achieved
        printf("Waiting for %d players...\n", playersAwaited);
        pthread_mutex_lock(&table->dealingCardsLock);
        while (table->cardsDealt != 1)
            pthread_cond_wait(&table->dealingCardsCond, &table->dealingCardsLock);
        printf("First one to play is player %d!\n", table->firstPlayerID);
        
        initialize_local_variables();
        
        // Get card hand dealt from player FIFO
        get_hand_from_fifo(fifoFD, handCards, HAND_CARDS);
        add_event("receive_cards", handCards , HAND_CARDS);
        
        pthread_mutex_unlock(&table->dealingCardsLock);
           
    }
    
    // Initialize the local table cards
    initialize_with_usedCard(tableCards, DECK_CARDS);

    // Open the interface and game synchronize threads
    pthread_create(&tidSync, NULL, game_sync, NULL);
    pthread_create(&tidInterface, NULL, game_interface, NULL);

    // Wait for the interface thread to exit, cancel the game sync if it hasn't been yet
    pthread_join(tidInterface, NULL);
    pthread_cancel(tidSync);

    printf("Press enter to leave\n");
    getchar();
    
    // Close player FIFO
    close_player_fifo(fifoFD, fifoName);

    // Close and unlink table_ready semaphore and destroy shared memory, if this process is dealer, just close
    // semaphore and unmap memory if regular player
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