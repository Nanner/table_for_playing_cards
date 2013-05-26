#include "player.h"

int create_player_fifo(char* fifoName) {
    int fd;
    
    if(mkfifo(fifoName, 0660) != 0) {
        perror("Problem making fifo");
        return -1;
    }
    
    if( (fd = open(fifoName, O_RDONLY | O_NONBLOCK)) == -1) {
        perror("Problem opening fifo");
        return -1;
    }
    
    return fd;
}

int open_player_fifo(char* fifoName) {
    int fd;
    
    do {
        fd = open(fifoName, O_WRONLY);
        if (fd == -1)
            sleep(1);
    }    while (fd == -1);
    
    return fd;
}

int close_player_fifo(int fd, char* fifoName) {
    
    close(fd); 
    
    if (unlink(fifoName) < 0) {
        printf("Error when destroying player FIFO %s\n", fifoName); 
        return -1;
    }
    
    return 0;
}

void get_hand_from_fifo(int fd, card_t cardHand[], int numberOfCards) {
    
    int n = read(fd, cardHand, numberOfCards * sizeof(card_t));
    
    if(n != numberOfCards * sizeof(card_t)) {
        printf("Problem receiving card hand from FIFO\n");
    }
    
}

void send_hand_to_fifo(char* fifoName, card_t cardHand[], int numberOfCards) {
    
    int fd = open_player_fifo(fifoName);
    
    write(fd, cardHand, numberOfCards * sizeof(card_t));
    
    close(fd);
    
}