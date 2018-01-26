#include <cstdio>
#include <math.h>
#include <unistd.h>

#ifdef SHAPE
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/shape.h>
#endif

#include <stdlib.h>

#include "card.h"
#include "freecell.h"
#include "option.h"
#include "undo.h"
#include "util.h"
#include "timeval.h"

#ifdef SHAPE
#include "cursor.bm"
#include "cursor_back.bm"
#include "boundingMask.bm"
#include "clipMask.bm"
#endif

// Typedef
enum MoveMode { SingleMode, MultipleMode };

// Extern declarations
extern Display* dpy;
extern Window gameWindow;

//
Card* hilighted = 0;
bool cursorChanged = false;
Cursor cursor;

#ifdef SHAPE
static bool initialized = false;
static Pixmap boundingMask;
static Pixmap clipMask;
#endif

static char bitmap[bmWidth * (cardHeight - 2)];
static MoveMode moveMode;

Card::Card(Suit s, unsigned int v)
  : NSWindow(true, gameWindow, 0, 0, cardWidth, cardHeight, 1, BlackPixel(dpy, 0))
{
#ifdef SHAPE
  if (Option::roundCard() && !initialized) {
    //Shape
    boundingMask = XCreateBitmapFromData(dpy, root(), (char*)boundingMask_bits,
                                         boundingMask_width,
                                         boundingMask_height);
    clipMask = XCreateBitmapFromData(dpy, root(), (char*)clipMask_bits,
                                     clipMask_width, clipMask_height);

    //Cursor
    XColor fore, back, xc;
    Pixmap p, mask;
    Colormap cm = DefaultColormap(dpy, 0);

    p =  XCreateBitmapFromData(dpy, gameWindow, (char*) cursor_bits,
                               cursor_width, cursor_height);
    mask = XCreateBitmapFromData(dpy, gameWindow, (char*) cursor_back_bits,
                                 cursor_back_width, cursor_back_height);
    XAllocNamedColor(dpy, cm, "white", &fore, &xc);
    XAllocNamedColor(dpy, cm, "black", &back, &xc);
    cursor = XCreatePixmapCursor(dpy, p,mask, &fore, &back, 0, 0);

    initialized = true;
  }
#endif

  _parent = 0;
  _stack = 0;
  _suit = s;
  _value = v;
  _removed = false;
  makeBitmap(s, v, bitmap);

  // initialization on X things
  unsigned long fore, back, hilight;

  if (suitColor(s) == RedSuit)
    fore = getColor(dpy, "red");
  else
    fore = getColor(dpy, "black");
  back = WhitePixel(dpy, 0);
  hilight = getColor(dpy, "lightskyblue4");

  _usualPixmap = XCreatePixmapFromBitmapData(dpy, gameWindow, bitmap,
                                             cardWidth - 2, cardHeight - 2,
                                             fore, back,
                                     DefaultDepth(dpy, DefaultScreen(dpy)));
  _hilightedPixmap = XCreatePixmapFromBitmapData(dpy, gameWindow, bitmap, cardWidth - 2,
                                                 cardHeight - 2, fore, hilight,
                                     DefaultDepth(dpy, DefaultScreen(dpy)));

  selectInput(ButtonPressMask | EnterWindowMask | LeaveWindowMask);
  backgroundPixmap(_usualPixmap);

#ifdef SHAPE
  if (Option::roundCard()) {
    XShapeCombineMask(dpy, window(), ShapeBounding, 0, 0, boundingMask, ShapeSet);
    XShapeCombineMask(dpy, window(), ShapeClip, 0, 0, clipMask, ShapeSet);
  }
#endif

  NSWindow::move(0, 0);

  map();
}

void Card::move(int dest_x, int dest_y, bool animate)
{
  const int steps = 100;

  timeout_t start_time, stop_time, mpause;

  // Time of end of animation
  get_time (&start_time);
  stop_time = start_time;
  incr_time (&stop_time, (unsigned) Option::speedup());

  // Delay between steps
  double_to_time ((double)Option::speedup() / 1000.0 / 2.0 / (double)steps,
                  &mpause);

  raise();

  if (animate && Option::animation()) {
    int oldx = x();
    int oldy = y();
    int newx = dest_x;
    int newy = dest_y;
    float curx = (float) oldx;
    float cury = (float) oldy;

    for (int i = 0; i < steps; i++) {
      curx += ((float) (newx - oldx)) / steps;
      cury += ((float) (newy - oldy)) / steps;
      NSWindow::move((int) curx, (int) cury);
      XFlush(dpy);
      add_time (&start_time, &mpause);
      wait_until (&start_time);
    }
  } else {
    NSWindow::move(dest_x, dest_y);
  }
  XFlush(dpy);
  raise();

  if (animate && Option::animation()) {
     wait_until (&stop_time);
  }
 
}

void Card::moveToStack(Stack* s, bool autoMoving, bool pushUndo)
{
  if (s == 0 || s == _stack) return;

  if (pushUndo)
    undoAddMove(_stack, s);

  _stack->popCard();

   moveToStackInitial(s, true);

  if (autoMoving && Option::autoPlay()) autoMove();
}

void Card::moveToStackInitial(Stack* s, bool animate)
{
  _stack = s;

  Card* top = _stack->topCard();
  if (top != 0 && top->canBeParent(this))
    parent(_stack->topCard());
  else
    parent(0);

  _stack->pushCard(this);
   move(_stack->next_x(), _stack->next_y(), animate);
}

bool Card::canBeParent(Card* c) const
{
  if (c == 0) return false;

  return (suitColor(c->suit()) != suitColor(_suit))
      && (_value == c->value() + 1);
}

void Card::dispatchEvent(const XEvent& ev)
{
  switch (ev.type) {
  case ButtonPress:
    dispatchButtonPress(ev);
    break;
  case EnterNotify:
    dispatchEnterNotify(ev);
    break;
  case LeaveNotify:
    dispatchLeaveNotify(ev);
    break;
  }
}

// Move to an empty single stack if possible
int Card::moveToSingleStack (void) {
   SingleStack* stack = emptySingleStack();
   if (stack != 0) {
      _stack->topCard()->moveToStack(stack);
      return true;
   } else {
     return false;
   }
}

void Card::dispatchButtonPress(const XEvent& ev)
{
  static Time lastPressTime = 0;

  if ( (hilighted == _stack->topCard()) && (ev.xbutton.button == 1) ) {
    // Second selection toggle hilight
    hilighted->unhilighten();
    hilighted = 0;
    if ( (ev.xbutton.time - lastPressTime < (unsigned) Option::doubleClick()) 
       && ! isSingleStack(_stack) ) {
      // Double click: Move to an empty single stack
      (void) this->moveToSingleStack();
    }
  } else if ( (hilighted != 0 && (hilighted->_stack == _stack) ) 
                              && (ev.xbutton.button == 1) ) {
    // Second selection on free cell: toggle hilight
    hilighted->unhilighten();
    hilighted = 0;
  } else if (hilighted == 0 && !_removed) {
    switch (ev.xbutton.button) {
    case 1:
      // First selection: toggle hilight
      _stack->topCard()->hilighten();
      hilighted = _stack->topCard();
      lastPressTime = ev.xbutton.time;
      break;
    case 2:
      // Middle button: Move to an empty single stack
      {
        SingleStack* stack = emptySingleStack();
        if (stack != 0)
          _stack->topCard()->moveToStack(stack);
      }
      break;
    case 3:
      // Righ button: Move to Done stack (if no autoplay) if possible
      //  otherwise move to Single stack if possible
      if (!moveToAppropriateDoneStack(_stack->topCard())) {
        (void) this->moveToSingleStack();
      }
      break;
    }
  } else if (cursorChanged && (ev.xbutton.button == 1) ) {
    // cursorChanged == true means moving is possible.
    if (moveMode == SingleMode) {
      // Move one card
      hilighted->unhilighten();
      hilighted->moveToStack(_stack);
      hilighted = 0;
      XUndefineCursor(dpy, window());
      cursorChanged = false;
    } else if (moveMode == MultipleMode) {
      // Move as many cards as possible 
      hilighted->unhilighten();
      moveMultipleCards(hilighted, _stack->topCard());
      hilighted = 0;
      XUndefineCursor(dpy, window());
      cursorChanged = false;
    } else {
      fprintf(stderr, "Bug in Card::dispatchButtonPress cursorChanged\n");
      exit(1);
    }
  }
}

void Card::dispatchEnterNotify(const XEvent&)
{
  if (hilighted == 0) return;

  if (_stack->acceptable(hilighted)) {
    XDefineCursor(dpy, window(), cursor);
    cursorChanged = true;
    moveMode = SingleMode;
  } else if (multipleMovable(hilighted, _stack->topCard())) {
    XDefineCursor(dpy, window(), cursor);
    cursorChanged = true;
    moveMode = MultipleMode;
  }
}

void Card::dispatchLeaveNotify(const XEvent&)
{
  if (cursorChanged) {
    XUndefineCursor(dpy, window());
    cursorChanged = false;
  }
}

// Interface to external
void resetHilighted()
{
  if (hilighted != 0) {
    hilighted->unhilighten();
    hilighted = 0;
  }
}

void initializeCursor()
{
  cursorChanged = false;
}

