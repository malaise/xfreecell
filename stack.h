#ifndef Stack_H
#define Stack_H

#include <vector>

#include "general.h"
#include "widget/widget.h"

class Card;

class Stack : public NSWindow {
public:
  Stack(int x_ini, int y_ini);
  virtual ~Stack() {}

  Card* topCard() const;
  unsigned int size() const { return _cards.size(); }
  int next_x() const { return _next_x; }
  int next_y() const { return _next_y; }

  virtual void pushCard(Card* c) { _cards.push_back(c); }
  virtual Card* popCard();
  virtual bool acceptable(Card*) const = 0;

  virtual void initialize();

protected:
  int _next_x, _next_y;

private:
  std::vector<Card*> _cards;
};

class PlayStack : public Stack {
public:
  PlayStack(int x_ini, int y_ini);

  void pushCard(Card*);
  Card* popCard();
  bool acceptable(Card*) const;

  void initialize();

  void dispatchEvent(const XEvent&);
  void dispatchButtonPress(const XEvent&);
  void dispatchEnterNotify();
  void dispatchLeaveNotify();
};

class SingleStack : public Stack {
public:
  SingleStack(int x_ini, int y_ini);

  Card* popCard();
  bool acceptable(Card*) const;

  void dispatchEvent(const XEvent&);
  void dispatchButtonPress();
  void dispatchEnterNotify();
  void dispatchLeaveNotify();
};

class DoneStack : public Stack {
public:
  DoneStack(int x_ini, int y_ini, Suit);

  Suit suit() const { return _suit; }

  void pushCard(Card*);
  Card* popCard();
  bool acceptable(Card*) const;

  void dispatchEvent(const XEvent&);

private:
  Suit _suit;
};

#endif
