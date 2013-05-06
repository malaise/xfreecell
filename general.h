#ifndef General_H
#define General_H

#include <errno.h>

static const char DefaultBackground[] = "forestgreen";
static const char VersionStr[] = "xfreecell 2.13";

const int mainWindowWidth = 670, mainWindowHeight = 700;

const unsigned int numCards = 52;
const unsigned int numPlayStack = 8;
const unsigned int numSingleStack = 4;
const unsigned int numDoneStack = 4;

const unsigned int cardWidth = 79;
const unsigned int cardHeight = 123;

const unsigned int bmWidth = (cardWidth - 2 + 7) / 8;

enum Suit {
  Heart, Diamond, Club, Spade
};

enum SuitColor {
  RedSuit, BlackSuit, ERROR
};

#endif
