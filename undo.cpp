#include <stack>

#include "card.h"
#include "stack.h"

// There is no need to make undo a class. It adds another 
// global variable, and I hate g v.

struct Move {
  Stack* from;
  Stack* to;

  Move(Stack* f, Stack* t) { from = f; to = t; }
  void doUndo();
  void doRedo();
};

void Move::doUndo()
{
  Card* c = to->topCard();

  c->moveToStack(from, false, false);
}

void Move::doRedo()
{
  Card* c = from->topCard();

  c->moveToStack(to, false, false);
}

std::stack<Move> moves;
std::stack<Move> undone;

static void undoClearUndone () {
  while (undone.size() > 0)
    undone.pop();
}

void undoClearMoves()
{
  undoClearUndone();
  while (moves.size() > 0)
    moves.pop();
}

void undoAddMove(Stack* from, Stack* to)
{
  undoClearUndone();
  Move m(from, to);
  moves.push(m);
}

void undoDoUndo()
{
  if (moves.size() == 0) return;

  Move &r = moves.top();

  Move m(r.from, r.to);
  undone.push(m);

  r.doUndo();
  moves.pop();
}

bool undoDoRedo()
{
  if (undone.size() == 0) return false;
  Move &r = undone.top();

  Move m(r.from, r.to);
  moves.push(m);

  r.doRedo();
  undone.pop();
  return true;
}

