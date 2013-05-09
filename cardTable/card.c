#include "card.h"

bool equal_cards(card_t card1, card_t card2) {
    return ( (card1.rank == card2.rank) && (card1.suit == card2.suit) );
}
