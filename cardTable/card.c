#include "card.h"

void start_deck(card_t deck[]) {

    int i;
    for (i = 0; i < DECK_CARDS; i++)
        deck[i] = i;

}

int deck_size(card_t deck[]) {
    int deckSize = 0;

    int i;
    for (i = 0; i < DECK_CARDS; i++)
        if (deck[i] != usedCard)
            deckSize++;

    return deckSize;
}

bool deck_empty(card_t deck[]) {
    return (deck_size(deck) > 0);
}

void shuffle_deck(card_t deck[]) {
    srand(time(NULL));
    int deckSize = deck_size(deck);

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

char* get_card_representation(card_t card) {
    char* representation = malloc(2 * sizeof (char));
    sprintf(representation, "%s%c", ranks[card / 4], suits[card % 4]);
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
    int cardsLeft = deck_size(deck);
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

    if (j = cardsToGive - 1)
        return true;
    else
        return false;
}