#include "player.h"

int create_player_fifo(char* fifoName) {
    int fd;
    
    if(mkfifo(fifoName, 0660) != 0) {
        perror("Problem making fifo.");
        return -1;
    }
    
    fd = open(fifoName, O_RDONLY);
    
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

void get_hand_from_fifo(int fd, card_t cardHand[]) {
    int n = 0;
    
    do {
        n = read(fd, cardHand, 1);
    }
    while (n>0);
}

void send_hand_to_fifo(int fd, card_t cardHand[], int numberOfCards) {
    
    write(fd, cardHand, numberOfCards * sizeof(card_t));
    
}