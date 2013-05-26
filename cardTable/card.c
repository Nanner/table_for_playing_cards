#include "card.h"

char *ranks[] = { "A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K" };
const char suits[] = { 'c', 'd', 'h', 's' };

const card_t usedCard = -1;

int compare_cards(const void* a, const void* b) {
    return ( *(int*)a - *(int*)b );
}

char getSuit(card_t card) {
    return suits[card / 13];
}

char* getRank(card_t card) {
    return ranks[card % 13];
}

void initialize_with_usedCard(card_t cards[], int numberOfCards) {
    int i;
    for (i = 0; i < numberOfCards; i++)
        cards[i] = usedCard;
}

void start_deck(card_t deck[]) {

    int i;
    for (i = 0; i < DECK_CARDS; i++)
        deck[i] = i;

}

int card_array_size(card_t cards[], int maxSize) {
    int deckSize = 0;

    int i;
    for (i = 0; i < maxSize; i++)
        if (cards[i] != usedCard)
            deckSize++;

    return deckSize;
}

bool deck_empty(card_t deck[]) {
    return (card_array_size(deck, DECK_CARDS) > 0);
}

void shuffle_deck(card_t deck[]) {
    srand((unsigned int)time(NULL));
    int deckSize = card_array_size(deck, DECK_CARDS);

    if (deckSize > 1) {
        int i;
        for (i = 0; i < deckSize - 1; i++) {
            int j = i + rand() / (RAND_MAX / (deckSize - i) + 1);
            card_t tmp = deck[j];
            deck[j] = deck[i];
            deck[i] = tmp;
        }
    }
}

void reorder_used_cards(card_t cards[], int numberOfCards) {
    
    card_t buf[numberOfCards];
    
    int i;
    int j = 0;
    for(i = 0; i < numberOfCards; i++) {
        if(cards[i] != usedCard) {
            buf[j] = cards[i];
            j++;
        }
    }
    
    for( ; j < numberOfCards; j++)
        buf[j] = usedCard;
    
    for(i = 0; i < numberOfCards; i++) {
        cards[i] = buf[i];
    }
    
}

void sort_cards(card_t cards[], int numberOfCards) {
    
    qsort(cards, numberOfCards, sizeof(card_t), compare_cards);
    
}

char* get_card_representation(card_t card) {
    char* representation = malloc(2 * sizeof (char));
    sprintf(representation, "%s%c", getRank(card), getSuit(card));
    return representation;
}

char* get_card_array_representation(card_t cards[], int numberOfCards) {
    char* representation = malloc(1024 * sizeof(char));
    
    if(numberOfCards == 0) {
        strcpy(representation, "-");
    }
    
    char currentSuit;
    int j;
    for(j = 0; j < numberOfCards; j++) {
        if(cards[j] != usedCard) {
            currentSuit = getSuit(cards[j]);
            break;
        }
    }
    
    int i;
    for (i = 0; i < numberOfCards; i++) {
        if (cards[i] != usedCard) {
            if( i > 0 ) {
                char suit = getSuit(cards[i]);
                if(suit != currentSuit) {
                    currentSuit = suit;
                    strcat(representation, "/");
                }
                else
                    strcat(representation, "-");
            }
            
            char* cardName = get_card_representation(cards[i]);
            if(i == 0)
                strcpy(representation, cardName);
            else
                strcat(representation, cardName);
            free(cardName);
        }
    }
    
    return representation;
}

char* get_unordered_card_array_representation(card_t cards[], int numberOfCards) {
    char* representation = malloc(1024 * sizeof(char));
    
    if(numberOfCards == 0) {
        strcpy(representation, "-");
    }
    
    int i;
    for (i = 0; i < numberOfCards; i++) {
        if (cards[i] != usedCard) {
            char* cardName = get_card_representation(cards[i]);
            if(i == 0)
                strcpy(representation, cardName);
            else
                strcat(representation, cardName);
            free(cardName);
            if (i != numberOfCards - 1)
                strcat(representation, "-");
        }
    }
    
    return representation;
}

void print_cards(card_t cards[], int numberOfCards) {
    char* representation = get_card_array_representation(cards, numberOfCards);
    printf("%s", representation);
    free(representation);
}

void print_unordered_cards(card_t cards[], int numberOfCards) {
    char* representation = get_unordered_card_array_representation(cards, numberOfCards);
    printf("%s", representation);
    free(representation);
}

bool give_hand(card_t deck[], card_t hand[], int cardsToGive) {
    int cardsLeft = card_array_size(deck, DECK_CARDS);
    if (cardsToGive > cardsLeft)
        return false;

    int cardsLeftToGive = cardsToGive;

    int i;
    int j = 0;
    for (i = DECK_CARDS - cardsLeft; cardsLeftToGive > 0 && i < DECK_CARDS; i++) {
        if (deck[i] != usedCard) {
            hand[j] = deck[i];
            deck[i] = usedCard;
            j++;
            cardsLeftToGive--;
        }
    }

    if (j == cardsToGive - 1)
        return true;
    else
        return false;
}

void copy_cards(card_t to[], card_t from[], int numberOfCards) {
    int i;
    for(i = 0; i < numberOfCards; i++)
        to[i] = from[i];
}