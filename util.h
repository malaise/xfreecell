#ifndef Util_H
#define Util_H

#include "general.h"
#include <X11/Xlib.h>

unsigned long getColor(Display*, const char*);
SuitColor suitColor(Suit);

void makeBitmap(Suit, int, char*);
void makeOneSymbolBitmap(Suit, char*);
#endif
