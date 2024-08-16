#ifndef Card_H
#define Card_H

#include "general.h"
#include "stack.h"
#include "widget/widget.h"

class Stack;

class Card : public NSWindow {
public:
  Card(Suit, unsigned int);

  void move_anim(int x, int y, bool animate = true);
  void initialize() { _stack = 0; _parent = 0; _removed = false; } //move(0, 0);
  void hilighten() const { backgroundPixmap(_hilightedPixmap); clear(); }
  void unhilighten() const { backgroundPixmap(_usualPixmap); clear(); }
  void moveToStack(Stack*, bool autoMoving = true, bool pushUndo = true);
  void moveToStackInitial(Stack*, bool animate = false);
  void get_parent(Card* c) { _parent = c; }
  void removed() { _removed = true; } // This is called only when pushed to DoneStack
  void undone() { _removed = false; } // This is called only when being undone and pushed back from DoneStack

  bool isTop() const { return _stack->topCard() == this; }
  bool canBeParent(Card* child_wanna_be) const;
  bool isRemoved() const { return _removed; }

  Suit suit() const { return _suit; }
  unsigned int value() const { return _value; }
  Stack* stack() const { return _stack; }
  Card* get_parent() const { return _parent; }

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
  int moveToSingleStack (void);
};

void resetHilighted();
void initializeCursor();
#endif
