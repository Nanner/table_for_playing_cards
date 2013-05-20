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
        strcat(creator, "Dealer");
        strcat(result, "-");
    }
    else if(strcmp(event.eventType, "gameover") == 0) {
        strcat(creator, "-");
        strcat(result, "-");
    }
    else if(strcmp(event.eventType, "table") == 0) {
        strcat(creator, "-");
        char* card_rep = get_card_array_representation(event.result, event.numberOfCardsInResult);
        strcat(result, card_rep);
        free(card_rep);
    }
    else {
        sprintf(creator, "Player%d", event.playerID);
        char* card_rep = get_card_array_representation(event.result, event.numberOfCardsInResult);
        strcat(result, card_rep);
        free(card_rep);
    }
    
    sprintf(creatorName, "%s-%s", creator, event.playerName);
    
    sprintf(representation, "%-20s|%-14s|%-14s|%s", timeStamp, creatorName, event.eventType, result);
    
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
        printf("\n");
    }
    
}
