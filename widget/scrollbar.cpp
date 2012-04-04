#include <stdlib.h>
#include "widget.h"

// ## NSVScrollbar ##
const unsigned int NSVScrollbar::_defaultWidth = 15;

NSVScrollbar::NSVScrollbar
(unsigned int bh, unsigned int h, NSScrollbarListener* nsl, void* nslArg)
  : NSPlate(down), _bar(up), listener(nsl), listenerArg(nslArg)
{
  NSWindow::width(_defaultWidth);
  height(h);
  selectInput(ButtonPressMask | ButtonReleaseMask | ButtonMotionMask | StructureNotifyMask);

  _movement = 1;

  _buttonPressed = false;

  _bar.parent(window());
  _bar.width(_defaultWidth - 2 * thickness());
  barHeight(bh);
  _bar.move(thickness(), thickness());
}

unsigned int NSVScrollbar::currentPos() const
{
  //return (_bar.y() - thickness()) * 100 / (height() - 2 * thickness());
  return _bar.y() - thickness();
}

void NSVScrollbar::barHeight(unsigned int arg)
{
  if ((unsigned int) thickness() * 2 < arg || arg <= (height() - 2 * thickness()))
    _bar.height(arg);
  else  if (arg <= (unsigned int) thickness() * 2)
    _bar.height(thickness() * 2 + 1);
  else
    _bar.height(height() - 2 * thickness());

}

inline bool NSVScrollbar::outOfBar(int arg)
  // Checks whether pointer is out of the bar or not.
{
  return (arg < _bar.y()) || (arg > _bar.y() + (int) _bar.height());
}

inline bool NSVScrollbar::inLimits(int arg)
  // Check whether bar can be moved to arg. arg will be ceiling of bar.
{
  int upperLimit = thickness();
  int lowerLimit = height() - thickness() - _bar.height();
  unsigned int delta = abs(arg - _bar.y());

  return (arg >= upperLimit && arg <= lowerLimit) && 
    (delta >= _movement && delta % _movement == 0);
}

inline int NSVScrollbar::appropriatePos(int arg)
{
  int delta = arg - _bar.y();

  return _bar.y() + (delta / (int) _movement) * _movement;
}

void NSVScrollbar::dispatchEvent(const XEvent& ev)
{
  switch (ev.type) {
  case ButtonPress:
    {
      int newY = appropriatePos(ev.xmotion.y - _bar.height() / 2);
      if (outOfBar(ev.xbutton.y) && inLimits(newY)) {
	int prev = _bar.y();
	_bar.y(newY);
	if (listener != 0)
	  listener->vScrollAction(ev, listenerArg, (_bar.y() - prev) / (int) _movement);
      } else if (insideBar(ev.xbutton.y)) {
	_buttonPressed = true;
      }
      break;
    }
  case ButtonRelease:
    _buttonPressed = false;
    break;
  case MotionNotify:
    if (_buttonPressed && inLimits(ev.xmotion.y - _bar.height() / 2)) {
      int prev = _bar.y();
      _bar.y(ev.xmotion.y - _bar.height() / 2);
      if (listener != 0)
	listener->vScrollAction(ev, listenerArg, (_bar.y() - prev) / (int) _movement);
    }
    break;
  }

  NSPlate::dispatchEvent(ev);
}

// ## NSHScrollbar ##
const unsigned int NSHScrollbar::_defaultHeight = 15;

NSHScrollbar::NSHScrollbar
(unsigned int bw, unsigned int w, NSScrollbarListener* nsl, void* nslArg)
  : NSPlate(down), _bar(up), listener(nsl), listenerArg(nslArg)
{
  NSWindow::height(_defaultHeight);
  width(w);
  selectInput(ButtonPressMask | ButtonReleaseMask | ButtonMotionMask | StructureNotifyMask);

  _movement = 1;

  _buttonPressed = false;

  _bar.parent(window());
  _bar.height(_defaultHeight - 2 * thickness());
  barWidth(bw);
  _bar.move(thickness(), thickness());
}

unsigned int NSHScrollbar::currentPos() const
{
  //  return (_bar.x() - thickness()) * 100 / (width() - 2 * thickness());
  return _bar.x() - thickness();
}

void NSHScrollbar::barWidth(unsigned int arg)
{
  if ((unsigned int) thickness() * 2 < arg || arg <= width() - thickness() * 2)
    _bar.width(arg);
  else if (arg <= (unsigned int) thickness() * 2)
    _bar.width(thickness() * 2 + 1);
  else
    _bar.width(width() - thickness() * 2);
}

inline bool NSHScrollbar::outOfBar(int arg)
  // Checks whether pointer is out of the bar or not.
{
  return (arg < _bar.x()) || (arg > _bar.x() + (int) _bar.width());
}

inline bool NSHScrollbar::inLimits(int arg)
  // Check whether bar can be moved to arg. arg will be ceiling of bar.
{
  int upperLimit = thickness();
  int lowerLimit = width() - thickness() - _bar.width();
  unsigned int delta = abs(arg - _bar.x());
  
  return (arg >= upperLimit && arg <= lowerLimit) &&
    (delta >= _movement && delta % _movement == 0);
}

inline int NSHScrollbar::appropriatePos(int arg)
{
  int delta = arg - _bar.x();

  return _bar.x() + (delta / (int) _movement) * _movement;
}

void NSHScrollbar::dispatchEvent(const XEvent& ev)
{
  switch (ev.type) {
  case ButtonPress:
    {
      int newX = appropriatePos(ev.xmotion.x - _bar.width() / 2);
      if (outOfBar(ev.xbutton.x) && inLimits(newX)) {
	int prev = _bar.x();
	_bar.x(newX);
	if (listener != 0)
	  listener->hScrollAction(ev, listenerArg, (_bar.x() - prev) / (int) _movement);
      } else if (insideBar(ev.xbutton.x)) {
	_buttonPressed = true;
      }
      break;
    }
  case ButtonRelease:
    _buttonPressed = false;
    break;
  case MotionNotify:
    if (_buttonPressed && inLimits(ev.xmotion.x - _bar.width() / 2)) {
      int prev = _bar.x();
      _bar.x(ev.xmotion.x - _bar.width() / 2);
      if (listener != 0)
	listener->hScrollAction(ev, listenerArg, (_bar.x() - prev) / (int) _movement);
    }
    break;
  }

  NSPlate::dispatchEvent(ev);
}
