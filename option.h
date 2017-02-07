#ifndef Option_H
#define Option_H

#include "widget/widget.h"

class Option : public NSFrame, public NSButtonListener, public NSTextListener {
public:
  Option();
  ~Option() { writePrefs(); }

  static void parse(int, char**);
  void waitForEvent();
  void hide();
  void buttonAction(const XEvent&, void*);
  void textAction(bool, void *);

  static int speedup() { return _speedup; }
  static bool queryWindow() { return _queryWindow; }
  static bool roundCard() { return _roundCard; }
  static bool animation() { return _animation; }
  static bool msSeed() { return _msSeed; }
  static bool autoPlay() { return _autoPlay; }
  static bool load() { return _load; }
  static std::string getDir() { return _dir; }

private:
  void readPrefs();
  void writePrefs();

  std::string saveFile;

  static int _speedup;
  static bool _queryWindow;
  static bool _roundCard;
  static bool _animation;
  static bool _msSeed;
  static bool _autoPlay;
  static bool _load;
  static std::string _dir;

  bool exitPressed;
  bool okPressed;

  NSVContainer mainCon;
  NSHContainer speedCon;
  NSLabel speedLabel;
  NSTextField speedTF;
  NSHContainer togglesCon;
  NSToggleButton anim, query, ms, play;
  NSHContainer replyCon;
  NSButton okButton, cancelButton;
};

#endif

