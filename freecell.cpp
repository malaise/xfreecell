#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "card.h"
#include "freecell.h"
#include "general.h"
#include "gnmanager.h"
#include "option.h"
#include "random.h"
#include "subwindows.h"
#include "undo.h"
#include "util.h"
#include "ms-compatible/MSNumbers.h"

#include "widget/widget.h"

// ##### Variables declarations #####
Display* dpy;
Window gameWindow;

static  Window toplevel;

// Stacks and cards
static PlayStack* playStack[numPlayStack];
static SingleStack* singleStack[numSingleStack];
static DoneStack* doneStack[numDoneStack];
static DoneStack *doneClub, *doneHeart, *doneDiamond, *doneSpade;

static Card* cards[4][13];

// Buttons
// NSInitialize() must be called before these.
static NSButton* newButton;
static NSButton* replayButton;
static NSButton* seedButton;
static NSButton* lostButton;
static NSButton* undoButton;
static NSButton* redoButton;
static NSButton* scoreButton;
static NSButton* prefButton;
static NSButton* aboutButton;
static NSButton* exitButton;

// subwindows
static Option* option;
static SeedWindow* seedWindow;
static ScoreWindow* scoreWindow;
static AboutWindow* aboutWindow;
static SingleOrMultiple* singleOrMultiple;
static AnotherOrQuitOrExit* anotherOrQuitOrExit;
static PlayAgainOrCancel* playAgainOrCancel;

const int hGap = 4;
const int vGap = 16;

// misc
static bool gamePlaying;
static int gameNumber;
static GameNumberManager* gnManager;
static const int PathLength = 256;

// ##### Functions declarations #####
static void adjustSubwindow(NSWindow*);
static bool gameWon();

static void distributeCards(unsigned int = 0);
static void redistributeCards(unsigned int = 0);
static void msDistributeCards(unsigned int = 0);

static void moveToDoneStackIfPossible(Card*);
static bool canAutoMove(Card*);

static void movePlayStackToPlayStack(Stack*, Stack*, unsigned int);

static void beginNewGame(int = 0);
static unsigned int numEmptySingleStacks();
static unsigned int numEmptyPlayStacks();
static PlayStack* emptyPlayStack();

static NSButtonCallback newCallback;
static NSButtonCallback replayCallback;
static NSButtonCallback seedCallback;
static NSButtonCallback lostCallback;
static NSButtonCallback undoCallback;
static NSButtonCallback redoCallback;
static NSButtonCallback scoreCallback;
static NSButtonCallback prefCallback;
static NSButtonCallback aboutCallback;
static NSButtonCallback exitCallback;

static void exitCb (const XEvent&) __attribute__ ((noreturn));
static void exitCb (const XEvent &ev) {
  exitCallback (ev, NULL);
}

// ##### Code #####
int main(int argc, char* argv[])
{

  NSInitialize();
  dpy = NSdisplay();

  // buttons
  newButton = new NSButton("New", 0, 0, &newCallback);
  replayButton = new NSButton("Replay", 0, 0, &replayCallback);
  seedButton = new NSButton("Seed", 0, 0, &seedCallback);
  lostButton = new NSButton("Lost", 0, 0, &lostCallback);
  undoButton = new NSButton("Undo", 0, 0, &undoCallback);
  redoButton = new NSButton("Redo", 0, 0, &redoCallback);
  scoreButton = new NSButton("Score", 0, 0, &scoreCallback);
  prefButton = new NSButton("Pref", 0, 0, &prefCallback);
  aboutButton = new NSButton("About", 0, 0, &aboutCallback);
  exitButton = new NSButton("Exit", 0, 0, &exitCallback);

  // toplevel
  unsigned long bgpixel = getColor(dpy, DefaultBackground);
  XSizeHints sh;
  NSStaticHContainer menuContainer;
  NSFrame menuFrame;

  // NSWidget is too powerless to make NSFrame gameWindow.
  toplevel = XCreateSimpleWindow(dpy, RootWindow(dpy, 0), 0, 0, mainWindowWidth,
                                 mainWindowHeight, 0, 0, bgpixel);
  XSelectInput(dpy, toplevel, StructureNotifyMask);
  XStoreName(dpy, toplevel, "xfreecell");
  XSetIconName(dpy, toplevel, "xfreecell");
  sh.max_width = mainWindowWidth;
  sh.max_height =  mainWindowHeight;
  sh.min_width = mainWindowWidth;
  sh.min_height = mainWindowHeight;
  sh.flags = PMaxSize|PMinSize;
  XSetWMNormalHints(dpy, toplevel, &sh);
  // XMapWindow(dpy, toplevel);
  NSRegisterExit(toplevel, exitCb);


  // menu
  menuContainer.add(newButton);
  menuContainer.add(replayButton);
  menuContainer.add(seedButton);
  menuContainer.add(lostButton);
  menuContainer.add(undoButton);
  menuContainer.add(redoButton);
  menuContainer.add(scoreButton);
  menuContainer.add(prefButton);
  menuContainer.add(aboutButton);
  menuContainer.add(exitButton);
  menuContainer.reallocate();
  menuFrame.container(&menuContainer);
  menuFrame.parent(toplevel);
  menuFrame.map();

  int menuHeight = menuFrame.height();

  // gameWindow
  gameWindow = XCreateSimpleWindow(dpy, toplevel, 0, menuHeight, mainWindowWidth,
                                   mainWindowHeight - menuHeight, 0, 0, bgpixel);
  XMapWindow(dpy, gameWindow);

  // subwindows
  option = new Option;
  seedWindow = new SeedWindow;
  scoreWindow = new ScoreWindow;
  aboutWindow = new AboutWindow;
  singleOrMultiple = new SingleOrMultiple;
  anotherOrQuitOrExit = new AnotherOrQuitOrExit;
  playAgainOrCancel = new PlayAgainOrCancel;

  adjustSubwindow(option);
  adjustSubwindow(seedWindow);
  adjustSubwindow(scoreWindow);
  adjustSubwindow(aboutWindow);
  adjustSubwindow(singleOrMultiple);
  adjustSubwindow(anotherOrQuitOrExit);
  adjustSubwindow(playAgainOrCancel);

  Option::parse(argc, argv); // this must be done after the creation of option.

  // stack
  for (unsigned int i = 0; i < numPlayStack; i++)
    playStack[i] = new PlayStack(i * (cardWidth + hGap) + hGap, 2 * vGap + cardHeight);
  for (unsigned int i = 0; i < numSingleStack; i++)
    singleStack[i] = new SingleStack((i + (numPlayStack / 2 - 2)) * (cardWidth + hGap) + hGap, vGap);
  doneStack[0] = new DoneStack(0 * (cardWidth + hGap) + hGap, vGap, Heart);
  doneHeart = doneStack[0];

  doneStack[1] = new DoneStack(1 * (cardWidth + hGap) + hGap, vGap, Spade);
  doneSpade = doneStack[1];

  doneStack[2] = new DoneStack((numPlayStack - 2) * (cardWidth + hGap) + hGap, vGap, Diamond);
  doneDiamond = doneStack[2];

  doneStack[3] = new DoneStack((numPlayStack - 1) * (cardWidth + hGap) + hGap, vGap, Club);
  doneClub = doneStack[3];

  // card
  for (int i = 0; i < 13; i++)
    (cards[0][i] = new Card(Heart, i))->moveToStackInitial(doneHeart);
  for (int i = 0; i < 13; i++)
    (cards[1][i] = new Card(Diamond, i))->moveToStackInitial(doneDiamond);
  for (int i = 0; i < 13; i++)
    (cards[2][i] = new Card(Club, i))->moveToStackInitial(doneClub);
  for (int i = 0; i < 13; i++)
    (cards[3][i] = new Card(Spade, i))->moveToStackInitial(doneSpade);

  // misc
  gamePlaying = false;
  gameNumber = 0;
  gnManager = new GameNumberManager;

  XMapWindow(dpy, toplevel);
  XSync(dpy, False);

  XEvent ev;
  while (true) {
    NSNextEvent(&ev);
    NSDispatchEvent(ev);

    if (gamePlaying && gameWon()) {
      scoreWindow->incWins();
      gnManager->addWonGame(gameNumber);
      undoClearMoves();
      anotherOrQuitOrExit->waitForEvent();

      if (anotherOrQuitOrExit->another()) {
        gamePlaying = true;
        redistributeCards();
      } else if (anotherOrQuitOrExit->quit()) {
        gamePlaying = false;
      } else {
        gamePlaying = false;
        break;
      }
    }
  }
  exitCb(ev); 
}

inline void adjustSubwindow(NSWindow* nsw)
{
  nsw->parent(gameWindow);
  nsw->move(mainWindowWidth / 2 - nsw->width() / 2,
            mainWindowHeight / 2 - nsw->height() / 2);
  nsw->borderWidth(1);
}

inline bool gameWon()
{
  return numEmptyPlayStacks() == numPlayStack;
}

inline int appropriateGameNumber()
{
  int gameNumber;

  srand(time(0));
  if (Option::msSeed()) {
    do {
      gameNumber = rand() % 32001;
    } while (gnManager->alreadyPlayed(gameNumber));
  } else {
    do {
      gameNumber = rand() % 100000; // I don't like too big number.
    } while (gnManager->alreadyPlayed(gameNumber));
  }

  return gameNumber;
}

inline void setWindowName(int num, bool ms)
{
  char line[30];

  if (ms)
    sprintf(line, "xfreecell %d (ms seed)", num);
  else
    sprintf(line, "xfreecell %d", num);

  XStoreName(dpy, toplevel, line);
  XSetIconName(dpy, toplevel, line);
}

void distributeCards(unsigned int gameNumber)
{
  if (gameNumber == 0)
    gameNumber = appropriateGameNumber();

  setWindowName(gameNumber, false);
  ::gameNumber = gameNumber;

  // Using the algorithm of MS Freecell
#ifdef BOGUSRANDOM
  NSsrand(gameNumber);
#else
  srand(gameNumber);
#endif

  Card* deck[numCards];

  for (unsigned int i = 0; i < 4; i++)
    for (unsigned int j = 0; j < 13; j++)
      deck[i * 13 + j] = cards[i][j];

  int wLeft = numCards;
  for (unsigned int i = 0; i < numCards; i++) {
    unsigned int j;
#ifdef BOGUSRANDOM
    j = NSrand() % wLeft;
#else
    j = rand() % wLeft;
#endif

    deck[j]->moveToStackInitial(playStack[(i % numPlayStack)]);
    deck[j] = deck[--wLeft];
  }
}

void redistributeCards(unsigned int gameNumber)
{
  for (unsigned int i = 0; i < numCards; i++) {
    cards[i / 13][i % 13]->initialize();
    cards[i / 13][i % 13]->unhilighten();
  }
  for (unsigned int i = 0; i < numPlayStack; i++)
    playStack[i]->initialize();
  for (unsigned int i = 0; i < numSingleStack; i++)
    singleStack[i]->initialize();
  for (unsigned int i = 0; i < numDoneStack; i++)
    doneStack[i]->initialize();
  initializeHilighted();
  initializeCursor();
  if (Option::msSeed())
    msDistributeCards(gameNumber);
  else
    distributeCards(gameNumber);
}

inline Card* numToCard(int num)
{
  static const int Club = 0;
  static const int Diamond = 1;
  static const int Heart = 2;
  static const int Spade = 3;

  int suit = (num - 1) % 4;

  switch (suit) {
  case Club:
    return cards[2][(num - 1) / 4];
  case Diamond:
    return cards[1][(num - 1) / 4];
  case Heart:
    return cards[0][(num - 1) / 4];
  case Spade:
    return cards[3][(num - 1) / 4];
  }

  fprintf(stderr, "Bug in numToCard\n");
  return 0;
}

bool msRead (char *fileName, char* line) {
  FILE* fp = fopen(fileName, "r");
  off_t size;

  if (fp == NULL) {
    perror("msDistributeCards fopen\n");
    fprintf(stderr, "Cannot open %s\n", fileName);
    return false;
  }

  if (fseek(fp, 0, SEEK_END) == -1) {
    perror("msDistributeCards fseek 1\n");
    fclose(fp);
    return false;
  }

  size = ftello (fp);
  if (size == -1) {
    perror("msDistributeCards ftell\n");
    fclose(fp);
    return false;
  } else if (size != 32001 * 52) {
    fprintf(stderr, "File %s has incorrect size\n", fileName);
    fclose(fp);
    return false;
  }

  if (fseek(fp, gameNumber * 52, SEEK_SET) == -1) {
    perror("msDistributeCards fseek 2\n");
    fclose(fp);
    return false;
  }

  if (fread(line, sizeof(char), 52, fp) != 52) {
    perror("msDistributeCards fread\n");
    fclose(fp);
    return false;
  }

  fclose(fp);
  return true;
}


void msDistributeCards(unsigned int gameNumber)
{
  if (gameNumber == 0 || gameNumber > 32000)
    gameNumber = appropriateGameNumber();

  ::gameNumber = gameNumber;
  setWindowName(gameNumber, true);

  char line[numCards];

  for (unsigned int i = 0; i < numCards; i++)
    line[i] = MSNumbers[gameNumber * numCards + i];

  Card* deck[numCards];

  for (unsigned int i = 1; i <= numCards; i++)
    deck[i-1] = numToCard(i);

  for (unsigned int i = 0; i < 4; i++) {
    for (unsigned int j = 0; j < 7; j++)
      deck[line[i*7+j]-1]->moveToStackInitial(playStack[i]);
  }

  for (unsigned int i = 0; i < 4; i++) {
    for (unsigned int j = 0; j < 6; j++)
      deck[line[i*6+j+28]-1]->moveToStackInitial(playStack[i+4]);
  }

}

//Callbacks
void newCallback(const XEvent&, void*)
{
  beginNewGame();
}

void replayCallback(const XEvent&, void*)
{
  beginNewGame(gameNumber);
}

void seedCallback(const XEvent&, void*)
{
  seedWindow->waitForEvent();
  if (seedWindow->ok()) {
    if (gnManager->alreadyPlayed(seedWindow->value())) {
      playAgainOrCancel->waitForEvent();
      if (playAgainOrCancel->cancel()) return;
    }

    beginNewGame(seedWindow->value());
  }
}

void lostCallback(const XEvent&, void*)
{
  int number;

  number = gnManager->randomLostGame();

  if (number == 0) return; // You cannot replay game number 0.

  beginNewGame(number);
}

void undoCallback(const XEvent&, void*)
{
  scoreWindow->incUndos();
  undoDoUndo();
}

void redoCallback(const XEvent&, void*)
{
  if (undoDoRedo()) scoreWindow->decUndos();
}

void scoreCallback(const XEvent&, void*)
{
  scoreWindow->waitForEvent();
}

void prefCallback(const XEvent&, void*)
{
  option->waitForEvent();
}

void aboutCallback(const XEvent&, void*)
{
  aboutWindow->waitForEvent();
}

void exitCallback(const XEvent&, void*) __attribute__ ((noreturn));
void exitCallback(const XEvent&, void*)
{
  if (gamePlaying) {
    scoreWindow->incDefeats();
    gnManager->addLostGame(gameNumber);
  }

  gnManager->writeFiles();
  exit(0);
}

// Functions for automove
void moveToDoneStackIfPossible(Card* card)
{
  if (card == 0) return;

  if (card->value() == 0 || canAutoMove(card))
    moveToAppropriateDoneStack(card);
}

void moveToAppropriateDoneStack(Card* card)
{
  switch (card->suit()) {
  case Heart:
    if (doneHeart->acceptable(card))
      card->moveToStack(doneHeart);
    break;
  case Diamond:
    if (doneDiamond->acceptable(card))
      card->moveToStack(doneDiamond);
    break;
  case Spade:
    if (doneSpade->acceptable(card))
      card->moveToStack(doneSpade);
    break;
  case Club:
    if (doneClub->acceptable(card))
      card->moveToStack(doneClub);
    break;
  default:
    fprintf(stderr, "Bug moveToAppropriateDoneStack\n");
    exit(1);
  }
}

bool canAutoMove(Card* card)
{
  if (suitColor(card->suit()) == RedSuit) {
    Card* topSpade = doneSpade->topCard();
    Card* topClub = doneClub->topCard();

    if (topSpade == 0 || topClub == 0)
      return false;
    else if (topClub->value() >= card->value() - 1 &&
             topSpade->value() >= card->value() - 1)
      return true;
    else
      return false;
  } else if (suitColor(card->suit()) == BlackSuit) {
    Card* topHeart = doneHeart->topCard();
    Card* topDiamond = doneDiamond->topCard();

    if (topHeart == 0 || topDiamond == 0)
      return false;
    else if (topHeart->value() >= card->value() - 1 &&
             topDiamond->value() >= card->value() - 1)
      return true;
    else
      return false;
  } else {
    fprintf(stderr, "Bug in canAutoMove\n");
    exit(1);
  }
}

// Functions for multipleMove
void movePlayStackToPlayStack(Stack* from, Stack* to, unsigned int num)
{
  if (from->size() < num) num = from->size();

  unsigned int numEmptySingleStack = numEmptySingleStacks();
  SingleStack* sStack;
  Card* movedCards[numSingleStack];
  unsigned int index = 0;

  if (numEmptySingleStack + 1 < num) num = numEmptySingleStack + 1;

  Card* tmp;
  while (index < numEmptySingleStack && num > 0) {
    sStack = emptySingleStack();
    tmp = from->topCard();
    tmp->moveToStack(sStack);
    movedCards[index++] = tmp;
    num--;
  }

  if (num != 0)
    from->topCard()->moveToStack(to);

  for (; index > 0; index--) {
    if (!movedCards[index - 1]->isRemoved())
      movedCards[index - 1]->moveToStack(to);
  }
}

// Util Functions
inline void beginNewGame(int gameNumber)
{
  if (gamePlaying) {
    scoreWindow->incDefeats();
    gnManager->addLostGame(::gameNumber);
  }

  gamePlaying = true;
  undoClearMoves();
  redistributeCards(gameNumber);
}

unsigned int numEmptySingleStacks()
{
  unsigned int num = 0;

  for (unsigned int i = 0; i < numSingleStack; i++)
    if (singleStack[i]->size() == 0) num++;

  return num;
}

unsigned int numEmptyPlayStacks()
{
  unsigned int num = 0;

  for (unsigned int i = 0; i < numPlayStack; i++)
    if (playStack[i]->size() == 0) num++;

  return num;
}

PlayStack* emptyPlayStack()
{
  for (unsigned int i = 0; i < numPlayStack; i++) {
    if (playStack[i]->size() == 0)
      return playStack[i];
  }

  return 0;
}

// Interface to external
void autoMove()
{
  Card* card;
  for (unsigned int i = 0; i < numPlayStack; i++) {
    card = playStack[i]->topCard();
    moveToDoneStackIfPossible(card);
  }
  for (unsigned int i = 0; i < numSingleStack; i++) {
    card = singleStack[i]->topCard();
    moveToDoneStackIfPossible(card);
  }
}

SingleStack* emptySingleStack()
{
  for (unsigned int i = 0; i < numSingleStack; i++) {
    if (singleStack[i]->size() == 0)
      return singleStack[i];
  }

  return 0;
}

bool multipleMovable(Card* from, Card* to)
{
  if (from == 0 || to == 0) return false;

  // Checks whether from's ancestors are accpeted
  Stack* destStack = to->stack();
  unsigned int movableNum = 1;
  bool found = false;

  for (Card* c = from; c != 0; c = c->parent(), movableNum++) {
    if (destStack->acceptable(c)) {
      found = true;
      break;
    }
  }

  if (!found) return false;

  // Checks whether there are enough spaces
  unsigned int numEmptySS = numEmptySingleStacks();
  unsigned int numEmptyPS = numEmptyPlayStacks();

  return movableNum <= (numEmptySS + 1) * (numEmptyPS + 1);
}

void moveMultipleCards(Card* from, Card* to)
{
  Stack* fromStack = from->stack();
  Stack* toStack = to->stack();

  int numCardsToBeMoved = to->value() - from->value();
  int numMovableCards = numEmptySingleStacks() + 1;
  int index = 0;
  PlayStack* tempStack[numPlayStack];
  int numEmptyPlayStack = numEmptyPlayStacks();

  PlayStack* tmp;
  for (; numEmptyPlayStack > 0 && numCardsToBeMoved > numMovableCards;
       numEmptyPlayStack--, numCardsToBeMoved -= numMovableCards) {
    tmp = emptyPlayStack();
    movePlayStackToPlayStack(fromStack, tmp, numMovableCards);
    tempStack[index++] = tmp;
  }

  if (numCardsToBeMoved > 1)
    movePlayStackToPlayStack(fromStack, toStack, numCardsToBeMoved);

  if (numCardsToBeMoved == 1)
    fromStack->topCard()->moveToStack(toStack);

  for (; index > 0; index--)
    movePlayStackToPlayStack(tempStack[index - 1], toStack, numMovableCards);
}

void moveMultipleCards(Card* from, PlayStack* destStack)
{
  Stack* fromStack = from->stack();

  unsigned int numMovableCards = 0;
  for (; from != 0; from = from->parent(), numMovableCards++)
    ;

  movePlayStackToPlayStack(fromStack, destStack, numMovableCards);
}

void mapSingleOrMultiple()
{
  singleOrMultiple->waitForEvent();
}

bool singleButtonPressed()
{
  return singleOrMultiple->single();
}

bool multipleButtonPressed()
{
  return singleOrMultiple->multiple();
}
