#ifndef Freecell_H
#define Freecell_H

void autoMove();
SingleStack* emptySingleStack();
bool multipleMovable(Card* from, Card* to);
void moveMultipleCards(Card* from, Card* to);
void moveMultipleCards(Card* from, PlayStack* to);
void mapSingleOrMultiple();
bool singleButtonPressed();
bool multipleButtonPressed();
void moveToAppropriateDoneStack(Card*);
#endif
