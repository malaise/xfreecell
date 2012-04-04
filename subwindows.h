#ifndef Subwindows_H
#define Subwindows_H

#include <stdlib.h>
#include "widget/widget.h"

class QueryWindow : public NSButtonListener, public NSFrame {
public:
  QueryWindow(const char*, const char*, const char*);
  
  bool left() const { return _left; }
  bool right() const { return _right; }

  void buttonAction(const XEvent&, void*);

  void waitForEvent();
private:
  NSLabel label;
  NSButton leftButton;
  NSButton rightButton;

  NSHContainer con;
  NSVContainer mainCon;

  bool _left;
  bool _right;
};

class SingleOrMultiple : public QueryWindow {
public:
  SingleOrMultiple() : QueryWindow("How many do you want to move?", "Single", "Multiple") {}
  
  bool single() const { return left(); }
  bool multiple() const { return right(); }
};

class AnotherOrQuit : public QueryWindow {
public:
  AnotherOrQuit() : QueryWindow("Another game?", "Yes", "No") {}

  bool another() const { return left(); }
  bool quit() const { return right(); }
};

class PlayAgainOrCancel : public QueryWindow {
public:
  PlayAgainOrCancel() : QueryWindow("You have played this game. Again?", "Yes", "No") {}
  
  bool again() const { return left(); }
  bool cancel() const { return right(); }
};

//##### ScoreWindow #####
  
class ScoreWindow : public NSFrame, public NSButtonListener {
public:
  ScoreWindow();
  ~ScoreWindow() { writeScore(); }
  
  void incWins();
  void incDefeats();
  void incUndos();
  void decUndos();

  void waitForEvent();

  void buttonAction(const XEvent&, void*);
    
private:
  void readScore();
  void writeScore() const;
  void setLabels();

  int _wins, _defeats;
  int _continuousWins;
  int _continuousDefeats;
  bool _previousWon;
  int _undos;
  int _totalScore;

  std::string saveFile;

  bool exitPressed;

  NSButton exitButton;
  NSLabel winLabel, defeatLabel, winpLabel, defeatpLabel, cwinLabel;
  NSLabel cdefeatLabel, undoLabel, totalLabel;
  NSVContainer mainCon;
};    

// ##### AboutWindow #####
class AboutWindow : public NSFrame, public NSButtonListener {
public:
  AboutWindow();

  void buttonAction(const XEvent&, void*);

  void waitForEvent();

private:
  bool okPressed;

  NSLabel version;
  NSLabel copyright;
  NSLabel homepage;
  NSButton okButton;
  NSVContainer labelCon, mainCon;
}; 

// ##### SeedWindow #####
class SeedWindow : public NSFrame, public NSButtonListener, public NSTextListener {
public:
  SeedWindow();

  void buttonAction(const XEvent&, void*);
  void textAction(bool, void *);

  int value() const { return atoi(textField.text()); }
  bool ok() const { return _okPressed; }
  void waitForEvent();

private:
  NSHContainer buttonCon;
  NSVContainer mainCon;
  NSLabel label;
  NSButton okButton, cancelButton;
  NSTextField textField;

  bool _okPressed, _cancelPressed;
};
#endif
