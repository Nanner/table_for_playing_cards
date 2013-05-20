#include "event.h"

char* get_event_representation(event_t event) {
    
    char* representation = malloc(MAX_LEN * sizeof(char)); //14 for type
    char creator[10]; //14
    char creatorName[80];
    char timeStamp[80];
    char result[500];
    
    struct tm * dateinfo;
    dateinfo = localtime(&event.timeStamp);
    
    strftime(timeStamp, 80, "%F %T", dateinfo); //20
    
    if(strcmp(event.eventType, "deal") == 0) {
        strcpy(creator, "Dealer");
        strcpy(result, "-");
        sprintf(creatorName, "%s-%s", creator, event.playerName);
    }
    else if(strcmp(event.eventType, "gameover") == 0) {
        strcpy(creatorName, "-");
        strcpy(result, "-");
    }
    else if(strcmp(event.eventType, "table") == 0) {
        strcpy(creatorName, "-");
        char* card_rep = get_unordered_card_array_representation(event.result, event.numberOfCardsInResult);
        strcpy(result, card_rep);
        free(card_rep);
    }
    else {
        sprintf(creator, "Player%d", event.playerID);
        char* card_rep = get_card_array_representation(event.result, event.numberOfCardsInResult);
        strcpy(result, card_rep);
        free(card_rep);
        sprintf(creatorName, "%s-%s", creator, event.playerName);
    }
    
    sprintf(representation, LOG_COLUMN_FORMAT, timeStamp, creatorName, event.eventType, result);
    
    return representation;
}

void print_event(event_t event) {
    
    char* eventRepresentation = get_event_representation(event);
    printf("%s", eventRepresentation);
    free(eventRepresentation);
    
}

void print_event_list(event_t events[], int numberOfEvents) {
    
    int i;
    for(i = 0; i < numberOfEvents; i++) {
        print_event(events[i]);
    }
    
}

void print_event_list_for_player(event_t events[], int numberOfEvents) {
    
    int i;
    for(i = 0; i < numberOfEvents; i++) {
        if(strcmp(events[i].eventType, "receive_cards") != 0) {
                print_event(events[i]);
        }
    }
    
}

void initialize_event_log(char * logname){
    char filename[MAX_LEN];
    strcpy(filename, logname);
    strcat(filename, LOG_FILE);
    int file = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (file == -1) {
  		perror(filename);
    }
    
    char buffer[MAX_LEN];
    sprintf(buffer, LOG_COLUMN_FORMAT, "when", "who", "what", "result");
    
    int len = (int)strlen(buffer);
    
    if ( write(file, buffer, len ) == -1){
        printf("Error writing to %s: %s\n", filename, strerror(errno));
    }
    
    close(file);
}

void write_to_log(char * logname, event_t event){
    char filename[MAX_LEN];
    strcpy(filename, logname);
    strcat(filename, LOG_FILE);
    int file = open(filename, O_WRONLY | O_APPEND, 0600);
    if (file == -1) {
  		perror(filename);
    }
    
    char* eventRepresentation = get_event_representation(event);
    int len = (int)strlen(eventRepresentation);
    if ( write(file, eventRepresentation, len) == -1){
        printf("Error writing to %s: %s\n", filename, strerror(errno));
    }
    free(eventRepresentation);
    
    close(file);
}
