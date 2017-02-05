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
void trace(const char*);
void trace1(const char*, const unsigned int);
void trace2(const char*, const unsigned int, const char*, const unsigned int);
#endif
