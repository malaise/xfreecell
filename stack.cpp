#include <cstdio>
#ifdef SHAPE
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/shape.h>
#endif

#include "card.h"
#include "freecell.h"
#include "general.h"
#include "option.h"
#include "stack.h"
#include "util.h"
#ifdef SHAPE
#include "boundingMask.bm"
#include "clipMask.bm"
#endif

extern Display* dpy;
extern Window gameWindow;
extern Card* hilighted;
extern bool cursorChanged;
extern Cursor cursor;

static unsigned int stackingHeight = 28;

Stack::Stack(int x_ini, int y_ini)
 : NSWindow(true, gameWindow, x_ini - 1, y_ini - 1)
{
  _next_x = x_ini; _next_y = y_ini; 
}

Card* Stack::topCard() const
{
  if (_cards.size() == 0) return 0;

  return _cards.back();
}

Card* Stack::popCard()
{
  if (_cards.size() == 0) return 0;

  Card* tmp = _cards.back();
  _cards.pop_back();
  return tmp;
}

void Stack::initialize()
{
  while (_cards.size() > 0) 
    _cards.pop_back();
}

// PlayStack
PlayStack::PlayStack(int x_ini, int y_ini)
  : Stack(x_ini, y_ini)
{
  resize(cardWidth, cardHeight * 20);
  background(getColor(dpy, DefaultBackground));
  selectInput(ButtonPressMask | EnterWindowMask | LeaveWindowMask);

  map();
}

void PlayStack::pushCard(Card* c)
{
  Stack::pushCard(c);
  _next_y += stackingHeight;
}

Card* PlayStack::popCard()
{
  if (size() == 0) return 0;
  _next_y -= stackingHeight;

  return Stack::popCard();
}

bool PlayStack::acceptable(Card* c) const
{
  if (size() == 0) return true;

  return topCard()->canBeParent(c);
}

void PlayStack::initialize()
{
  _next_y -= stackingHeight * size();
  Stack::initialize();
}

void PlayStack::dispatchEvent(const XEvent& ev)
{
  switch (ev.type) {
  case ButtonPress:
    dispatchButtonPress(ev);
    break;
  case EnterNotify:
    dispatchEnterNotify();
    break;
  case LeaveNotify:
    dispatchLeaveNotify();
    break;
  }
}

void PlayStack::dispatchButtonPress(const XEvent& ev)
{
  if (hilighted == 0 || size() != 0 || !cursorChanged) return;
  
  if (Option::queryWindow()) {
    if (hilighted->parent() == 0) { //single
      hilighted->unhilighten();
      hilighted->moveToStack(this);
      hilighted = 0;
      XUndefineCursor(dpy, window());
      cursorChanged = false;
    } else {
      mapSingleOrMultiple();
      if (singleButtonPressed()) { //single
	hilighted->unhilighten();
	hilighted->moveToStack(this);
	hilighted = 0;
	XUndefineCursor(dpy, window());
	cursorChanged = false;
      } else if (multipleButtonPressed()) { //multiple
	hilighted->unhilighten();
	moveMultipleCards(hilighted, this);
	hilighted = 0;
	XUndefineCursor(dpy, window());
	cursorChanged = false;
      } else 
	fprintf(stderr, "Error in PlayStack::dispatchButtonPress()\n");
    }
  } else {
    switch (ev.xbutton.button) {
    case 1: // single
      hilighted->unhilighten();
      hilighted->moveToStack(this);
      XUndefineCursor(dpy, window());
      cursorChanged = false;
      hilighted = 0;
      break;
    case 3: 
      if (hilighted->parent() == 0) { //single
	hilighted->unhilighten();
	hilighted->moveToStack(this);
	hilighted = 0;
	XUndefineCursor(dpy, window());
	cursorChanged = false;
      } else {
	hilighted->unhilighten();
	moveMultipleCards(hilighted, this);
	XUndefineCursor(dpy, window());
	cursorChanged = false;
	hilighted = 0;
      }
      break;
    }
  }
}

void PlayStack::dispatchEnterNotify()
{
  if (hilighted != 0 && size() == 0) {
    XDefineCursor(dpy, window(), cursor);
    cursorChanged = true;
  }
}

void PlayStack::dispatchLeaveNotify()
{
  if (cursorChanged) {
    XUndefineCursor(dpy, window());
    cursorChanged = false;
  }
}

//SingleStack
SingleStack::SingleStack(int x_ini, int y_ini)
  : Stack(x_ini, y_ini)
{
  resize(cardWidth, cardHeight);
  background(getColor(dpy, "darkgreen"));
  border(getColor(dpy, "green"));
  borderWidth(1);
  selectInput(ButtonPressMask);

  map();
}

Card* SingleStack::popCard()
{
  if (size() == 0) return 0;

  return Stack::popCard();
}

bool SingleStack::acceptable(Card* c __attribute__ ((unused)) ) const
{
  if (size() == 0) return true;
  
  return false;
}

void SingleStack::dispatchEvent(const XEvent& ev)
{
  if (ev.type != ButtonPress || hilighted == 0) return;
  
  if (acceptable(hilighted)) {
    hilighted->unhilighten();
    hilighted->moveToStack(this);
    hilighted = 0;
  }
}

//DoneStack
#ifdef SHAPE
static Pixmap clipMask = 0;
static Pixmap boundingMask = 0;
static bool initialized = false;
#endif
static char bitmap[bmWidth * (cardHeight - 2)];

DoneStack::DoneStack(int x_ini, int y_ini, Suit s)
  : Stack(x_ini, y_ini)
{
#ifdef SHAPE
  if (Option::roundCard() && !initialized) {
    boundingMask = XCreateBitmapFromData(dpy, RootWindow(dpy, 0),  (char*) boundingMask_bits,
                                         boundingMask_width, boundingMask_height);
    clipMask = XCreateBitmapFromData(dpy, RootWindow(dpy, 0), (char*) clipMask_bits,
                                     clipMask_width, clipMask_height);
    initialized = true;
  }
#endif

  _suit = s;


  unsigned long fore, back;
  Pixmap bgpixmap;

  if (suitColor(s) == RedSuit)
    fore = getColor(dpy, "red");
  else
    fore = getColor(dpy, "black");
  back = WhitePixel(dpy, 0);
  makeOneSymbolBitmap(s, bitmap);

  bgpixmap = XCreatePixmapFromBitmapData(dpy, gameWindow, bitmap, cardWidth - 2,
					 cardHeight - 2, fore, back, DefaultDepth(dpy, DefaultScreen(dpy)));
  resize(cardWidth, cardHeight);
  backgroundPixmap(bgpixmap);
  border(getColor(dpy, "black"));
  borderWidth(1);
  selectInput(ButtonPressMask);

#ifdef SHAPE
  if (Option::roundCard()) {
    XShapeCombineMask(dpy, window(), ShapeBounding, 0, 0, boundingMask, ShapeSet);
    XShapeCombineMask(dpy, window(), ShapeClip, 0, 0, clipMask, ShapeSet);
  }
#endif

  map();
}

void DoneStack::pushCard(Card* c)
{
  c->removed();
  Stack::pushCard(c);
}

Card* DoneStack::popCard()
{
  if (size() == 0) return 0;
  //  fprintf(stderr, "Can't pop\n");
  //  exit(1);

  Card *c = Stack::popCard(); 
  c->undone();
  return c;
}

bool DoneStack::acceptable(Card* c) const
{
  if (c == 0) return false;

  if (size() == 0) {
    if ((c->suit() == _suit) && (c->value() == 0)) // Ace
      return true;
    else
      return false;
  }

  if ((c->suit() == _suit) && (c->value() == topCard()->value() + 1))
    return true;

  return false;
}

void DoneStack::dispatchEvent(const XEvent& ev)
{
  if (ev.type != ButtonPress || hilighted == 0) return;

  if (acceptable(hilighted)) {
    hilighted->unhilighten();
    hilighted->moveToStack(this);
    hilighted = 0;
  }
}
