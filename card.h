#ifndef Card_H
#define Card_H

#include "general.h"
#include "stack.h"
#include "widget/widget.h"

class Stack;

class Card : public NSWindow {
public:
  Card(Suit, unsigned int);

  void move(int x, int y, bool animate = true);
  void initialize() { _stack = 0; _parent = 0; _removed = false; } //move(0, 0);
  void hilighten() const { backgroundPixmap(_hilightedPixmap); clear(); }
  void unhilighten() const { backgroundPixmap(_usualPixmap); clear(); }
  void moveToStack(Stack*, bool autoMove = true, bool pushUndo = true);
  void moveToStackInitial(Stack*);
  void parent(Card* c) { _parent = c; }
  void removed() { _removed = true; } // This is called only when pushed to DoneStack
  void undone() { _removed = false; } // This is called only when being undone and pushed back from DoneStack

  bool isTop() const { return _stack->topCard() == this; }
  bool canBeParent(Card* child_wanna_be) const;
  bool isRemoved() const { return _removed; }

  Suit suit() const { return _suit; }
  unsigned int value() const { return _value; }
  Stack* stack() const { return _stack; }
  Card* parent() const { return _parent; }

  void dispatchEvent(const XEvent&);
  void dispatchButtonPress(const XEvent&);
  void dispatchEnterNotify(const XEvent&);
  void dispatchLeaveNotify(const XEvent&);

private:
  Pixmap _usualPixmap, _hilightedPixmap;

  Suit _suit;
  unsigned int _value;
  Stack* _stack;
  Card* _parent;

  bool _removed;
};

void initializeHilighted();
void initializeCursor();
#endif
