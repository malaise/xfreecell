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
static NSButton* saveButton;
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
static bool traceOn;

// ##### Functions declarations #####
static void adjustSubwindow(NSWindow*);
static bool gameWon();

static void resetCards(void);
static void distributeCards(unsigned int = 0);
static void msDistributeCards(unsigned int = 0);
static void redistributeCards(unsigned int = 0);

static void moveToDoneStackIfPossible(Card*);
static bool canAutoMove(Card*);

static void movePlayStackToPlayStack(Stack*, Stack*, unsigned int);

static void beginNewGame(int = 0);
static void load(void);
static unsigned int numEmptySingleStacks();
static unsigned int numEmptyPlayStacks();
static PlayStack* emptyPlayStack(Stack*);

static NSButtonCallback newCallback;
static NSButtonCallback replayCallback;
static NSButtonCallback seedCallback;
static NSButtonCallback lostCallback;
static NSButtonCallback undoCallback;
static NSButtonCallback redoCallback;
static NSButtonCallback saveCallback;
static NSButtonCallback scoreCallback;
static NSButtonCallback prefCallback;
static NSButtonCallback aboutCallback;
static NSButtonCallback exitCallback;

static void exitCb (const XEvent&) __attribute__ ((noreturn));
static void exitCb (const XEvent &ev) {
  exitCallback (ev, NULL);
}

// ##### Code #####
void trace(const char* msg) {
  if (!traceOn) return;
  printf ("%s\n", msg);
}

void trace1(const char* msg, const unsigned int a1) {
  if (!traceOn) return;
  printf ("%s %u\n", msg, a1);
}

void trace2(const char* msg1, const unsigned int a1, 
            const char* msg2, const unsigned int a2){
  if (!traceOn) return;
  printf ("%s %u %s %u\n", msg1, a1, msg2, a2);
}



int main(int argc, char* argv[])
{
  traceOn = (getenv("xfreecell_TRACE") != 0);

  NSInitialize();
  dpy = NSdisplay();

  // buttons
  newButton = new NSButton("New", 0, 0, &newCallback);
  replayButton = new NSButton("Replay", 0, 0, &replayCallback);
  seedButton = new NSButton("Seed", 0, 0, &seedCallback);
  lostButton = new NSButton("Lost", 0, 0, &lostCallback);
  undoButton = new NSButton("Undo", 0, 0, &undoCallback);
  redoButton = new NSButton("Redo", 0, 0, &redoCallback);
  saveButton = new NSButton("Save", 0, 0, &saveCallback);
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
  menuContainer.add(saveButton);
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
  gnManager = new GameNumberManager(option->getDir());

  // Load
  if (option->load()) load();

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
      gamePlaying = false;
      anotherOrQuitOrExit->waitForEvent();

      if (anotherOrQuitOrExit->another()) {
        gamePlaying = true;
        redistributeCards();
      } else if (anotherOrQuitOrExit->quit()) {
        ;
      } else if (anotherOrQuitOrExit->exit()) {
        break;
      }
    }
  }
  exitCb(ev); 
}

// Hide any sub-window
void hideWindows() {
  option->hide();
  anotherOrQuitOrExit->hide();
  singleOrMultiple->hide();
  playAgainOrCancel->hide();
  scoreWindow->hide();
  aboutWindow->hide();
  seedWindow->hide();
}

void adjustSubwindow(NSWindow* nsw)
{
  nsw->parent(gameWindow);
  nsw->move(mainWindowWidth / 2 - nsw->width() / 2,
            mainWindowHeight / 2 - nsw->height() / 2);
  nsw->borderWidth(1);
}

bool gameWon()
{
  return numEmptyPlayStacks() == numPlayStack;
}

int appropriateGameNumber()
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

void setWindowName(int num, bool ms)
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

void resetCards(void)
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
  resetHilighted();
  initializeCursor();
}

void redistributeCards(unsigned int gameNumber)
{
  resetCards();
  if (Option::msSeed()) msDistributeCards(gameNumber);
  else distributeCards(gameNumber);
}

static const int iClub = 0;
static const int iDiamond = 1;
static const int iHeart = 2;
static const int iSpade = 3;
inline Card* numToCard(int num)
{

  int suit = (num - 1) % 4;

  switch (suit) {
    case iClub:
      return cards[2][(num - 1) / 4];
    case iDiamond:
      return cards[1][(num - 1) / 4];
    case iHeart:
      return cards[0][(num - 1) / 4];
    case iSpade:
      return cards[3][(num - 1) / 4];
  }

  fprintf(stderr, "Bug in numToCard\n");
  return 0;
}

inline unsigned int cardToNum (const Card* card) {
  int res;

  switch (card->suit()) {
    case Club:
      res = iClub;
    break;
    case Diamond:
      res = iDiamond;
    break;
    case Heart:
      res = iHeart;
    break;
    case Spade:
      res = iSpade;
    break;
  }
  return res + 4 * card->value() + 1;
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
  resetHilighted();
  scoreWindow->incUndos();
  undoDoUndo();
}

void redoCallback(const XEvent&, void*)
{
  resetHilighted();
  if (undoDoRedo()) scoreWindow->decUndos();
}

static const std::string saveName="/saved";
static FILE* saveFile;
static void writeSave (const int i, const int len = 1) {
  fwrite (&i, len, 1, saveFile);
}
static int readSave (const int len = 1) {
  int r;
  fread (&r, len, 1, saveFile);
  return r;
}

void saveCallback(const XEvent&, void*)
{
  const std::string name=option->getDir() + saveName;
  Card *card;
  unsigned int i, j;
  unsigned int u;

  // Open file and save game number
  saveFile = fopen(name.c_str(), "w");
  writeSave (gameNumber, 4);

  // The 4 single stack
  for (i = 0; i < numSingleStack; i++) {
    card = singleStack[i]->topCard();
    writeSave (card == 0 ? 0 : cardToNum (card));
  }

  // The 8 play stacks, each terminated by a 0
  for (i = 0; i < numPlayStack; i++) {
    j = playStack[i]->size();
    for (;;) {
      j--;
      card = playStack[i]->nthCard(j);
      if (card == 0) break;
      u = cardToNum (card);
      writeSave (u);
    }
    // Mark end of playstack
    writeSave (0);
  }

  //The 4 done stacks, each terminated by a 0
  for (i = 0; i < numDoneStack; i++) {
    j = doneStack[i]->size();
    for (;;) {
      j--;
      card = doneStack[i]->nthCard(j);
      if (card == 0) break;
      u = cardToNum (card);
      writeSave (u);
    }
    // Mark end of playstack
    writeSave (0);
  }

  fclose (saveFile);
}

void scoreCallback(const XEvent&, void*)
{
  hideWindows();
  scoreWindow->waitForEvent();
}

void prefCallback(const XEvent&, void*)
{
  hideWindows();
  option->waitForEvent();
}

void aboutCallback(const XEvent&, void*)
{
  hideWindows();
  aboutWindow->waitForEvent();
}

void exitCallback(const XEvent&, void*) __attribute__ ((noreturn));
void exitCallback(const XEvent&, void*)
{
  if (gamePlaying) {
    scoreWindow->incDefeats();
    gnManager->addLostGame(gameNumber);
  }

  hideWindows();
  gnManager->writeFiles();
  exit(0);
}

void beginNewGame(int gameNumber)
{
  if (gamePlaying) {
    scoreWindow->incDefeats();
    gnManager->addLostGame(::gameNumber);
  }

  hideWindows();
  gamePlaying = true;
  undoClearMoves();
  redistributeCards(gameNumber);
}

void load (void) {
  const std::string name=option->getDir() + saveName;
  Card *card;
  unsigned int i;
  int v;
  
  // Reset cards
  resetCards();

  // Open file and read game number
  trace ("Start load");
  saveFile = fopen(name.c_str(), "r");
  gameNumber = readSave(4);
  if (gameNumber != 0) setWindowName(gameNumber, false);
  hideWindows();
  gamePlaying = (gameNumber != 0);

  // The 4 single stack
  trace ("  Load single stacks");
  for (i = 0; i < numSingleStack; i++) {
    v = readSave();
    if (v != 0) {
      card = numToCard(v);
      card->moveToStackInitial(singleStack[i]);
    }
  }

  // The 8 play stacks, each terminated by a 0
  trace ("  Load play stacks");
  for (i = 0; i < numPlayStack; i++) {
    for (;;) {
      v = readSave();
      if (v == 0) break;
      card = numToCard(v);
      card->moveToStackInitial(playStack[i]);
    }
  }

  //The 4 done stacks, each terminated by a 0
  trace ("  Load done stacks");
  for (i = 0; i < numDoneStack; i++) {
    for (;;) {
      v = readSave();
      if (v == 0) break;
      card = numToCard(v);
      card->moveToStackInitial(doneStack[i]);
    }
  }

  fclose (saveFile);
  trace ("Done load");
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

// Interface to external
// Move as many cards as possible to Done stacks
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

// Functions for multipleMove
// Move num cards from a stack to another
static void movePlayStackToPlayStack(Stack* from, Stack* to, unsigned int num)
{
  unsigned int numEmptySingStack = numEmptySingleStacks();
  unsigned int numEmptyPlayStack = numEmptyPlayStacks();
  Stack* tmpStack;
  Card* movedCards[numSingleStack + numPlayStack];
  unsigned int index = 0;

  // Move num-1 cards into empty single stacks then empty play stacks
  // Store them in moveCards
  Card* tmpCard;
  while (index < numEmptySingStack && num > 1) {
    tmpStack = emptySingleStack();
    tmpCard = from->topCard();
    tmpCard->moveToStack(tmpStack);
    movedCards[index++] = tmpCard;
    num--;
  }
  while ((index < numEmptySingStack + numEmptyPlayStack) && num > 1) {
    tmpStack = emptyPlayStack((Stack*)to);
    tmpCard = from->topCard();
    tmpCard->moveToStack(tmpStack);
    movedCards[index++] = tmpCard;
    num--;
  }

  // Move num-th card into dest stack
  if (num != 0) {
    if (from->topCard()->isRemoved()) {
      return;
    }
    from->topCard()->moveToStack(to);
  }

  // Move cards from tmp stacks into dest stack
  for (; index > 0; index--) {
    if (movedCards[index - 1]->isRemoved()) {
      return;
    }
      movedCards[index - 1]->moveToStack(to);
  }
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

// Find the first empty play stack - skip avoid
PlayStack* emptyPlayStack(Stack *avoid)
{
  for (unsigned int i = 0; i < numPlayStack; i++) {
    if ( (playStack[i]->size() == 0) && (playStack[i] != avoid) ) {
      return playStack[i];
    }
  }

  return 0;
}

// Find the first empty single stack
SingleStack* emptySingleStack()
{
  for (unsigned int i = 0; i < numSingleStack; i++) {
    if (singleStack[i]->size() == 0)
      return singleStack[i];
  }

  return 0;
}

inline unsigned int min(unsigned int x, unsigned int y) {
  return (x < y ? x : y);
}

// Check if card "from" can be moved on top of card "to" with multiple moves
bool multipleMovable(Card* from, Card* to)
{

  // Checks whether from's ancestors are accepted
  Stack* destStack = to->stack();
  unsigned int toMoveNum = 1;
  unsigned int movableNum = 1;
  bool found = false;

  if (from == 0 || to == 0) return false;

  for (Card* c = from; c != 0; c = c->parent(), toMoveNum++) {
    if (destStack->acceptable(c)) {
      found = true;
      break;
    }
  }

  if (!found) return false;

  // Checks whether there are enough spaces
  unsigned int numEmptySS = numEmptySingleStacks();
  unsigned int numEmptyPS = numEmptyPlayStacks();

  // Algo will put a card on each Play then on each Single stack
  //  then stack all cards on the first P... and so on
  //  then put a card in each S
  // if T=P+S, then Movable=T*(T+1)/2 - S*(S+1)/2 + S + 1
  //                       = P + S + (P*P+P)/2 + S + 1
  movableNum = numEmptyPS * numEmptySS 
             + (numEmptyPS * numEmptyPS + numEmptyPS) / 2
             + numEmptySS + 1;

  return toMoveNum <= movableNum;
}

// Multi move card "from" on top of card "to"
void moveMultipleCards(Card* from, Card* to)
{
  PlayStack* toStack = (PlayStack*)to->stack();
  moveMultipleCards(from, toStack);
}

// Multi move card "from" on top of stack destStack
void moveMultipleCards(Card* from, PlayStack* toStack)
{
  Stack* fromStack = from->stack();
  unsigned int numEmptyPlayStack = numEmptyPlayStacks();
  unsigned int numEmptySingStack = numEmptySingleStacks();
  // Nb of cards that can be moved at each iteration
  //  according to empty stacks
  unsigned int numMovableCards;
  // Nb of cards that can need to moved according to src and dest
  unsigned int numCardsToBeMoved;


  if (toStack->size() == 0) {
    // Move to empty play stack
    // Potentially all from stack
    numCardsToBeMoved = 0;
    while (from != 0) {
      from = from->parent();
      numCardsToBeMoved++;
    }
    // One less play stack to be used as tmp
    numEmptyPlayStack--;
  } else {
    // Move to a top of a card
    numCardsToBeMoved = toStack->topCard()->value() - from->value();
  }
  numMovableCards = numEmptySingStack + numEmptyPlayStack;
  trace2 ("Movable", numMovableCards, " To move", numCardsToBeMoved);


  // Total Nb of cards to move
  // List of playstack used
  PlayStack* tempStack[numPlayStack];
  int index = 0;
  unsigned int num;

  // Move cards into each empty play stack
  PlayStack* tmp;
  while (numEmptyPlayStack > 0 && numCardsToBeMoved > numEmptySingStack) {
    num = min(numCardsToBeMoved - 1, numMovableCards);
    tmp = emptyPlayStack(toStack);
    tempStack[index++] = tmp;
    trace1 ("  Moving to play", num);
    movePlayStackToPlayStack(fromStack, tmp, num);
    trace1 ("  Moved ", tempStack[index - 1]->size());
    numEmptyPlayStack--;
    numCardsToBeMoved -= num;
    // One less free play stack
    numMovableCards--;
  }

  if (numCardsToBeMoved > 1) {
    // Move remaining cards card to dest, using single stacks
    trace1 ("  Moving last cards to dest", numCardsToBeMoved);
    movePlayStackToPlayStack(fromStack, toStack, numCardsToBeMoved);
  } else if (numCardsToBeMoved == 1) {
    // Move final card to dest
    trace ("  Moving last card to dest");
    fromStack->topCard()->moveToStack(toStack);
  }

  // Move cards from used play stacks to dest
  trace1 ("  Movable", numMovableCards);
  for (; index > 0; index--) {
    trace1 ("  index", index);
    numCardsToBeMoved = tempStack[index - 1]->size();
    trace2 ("  Movable tmp", numMovableCards, " To move", numCardsToBeMoved);
    movePlayStackToPlayStack(tempStack[index - 1], toStack, numCardsToBeMoved);
    numMovableCards++;
  }
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
