#include "card.h"

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

void reorder_cards(card_t cards[], int numberOfCards) {
    
    card_t buf[numberOfCards];
    
    int i;
    int j = 0;
    for(i = 0; i < numberOfCards; i++) {
        if(cards[i] != usedCard) {
            buf[j] = cards[i];
            j++;
        }
    }
    
    for(j; j < numberOfCards; j++)
        buf[j] = usedCard;
    
    for(i = 0; i < numberOfCards; i++) {
        cards[i] = buf[i];
    }
    
}

char* get_card_representation(card_t card) {
    char* representation = malloc(2 * sizeof (char));
    sprintf(representation, "%s%c", ranks[card / 4], suits[card % 4]);
    return representation;
}

char* get_card_array_representation(card_t cards[], int numberOfCards) {
    char* representation = malloc(1024 * sizeof(char));
    
    int i;
    for (i = 0; i < numberOfCards; i++) {
        if (cards[i] != usedCard) {
            char* cardName = get_card_representation(cards[i]);
            strcat(representation, cardName);
            free(cardName);
            if (i != numberOfCards - 1)
                strcat(representation, "-");
        }
    }
    
    return representation;
}

void print_cards(card_t cards[], int numberOfCards) {
    int i;
    for (i = 0; i < numberOfCards; i++) {
        if (cards[i] != usedCard) {
            char* cardName = get_card_representation(cards[i]);
            printf("%s", cardName);
            free(cardName);
            if (i != numberOfCards - 1)
                printf("-");
        }
    }
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