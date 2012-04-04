#ifndef Undo_H
#define Undo_H
void undoClearMoves();
void undoAddMove(Stack* from, Stack* to);
void undoDoUndo();
bool undoDoRedo();
#endif
