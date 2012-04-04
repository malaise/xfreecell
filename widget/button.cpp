#include "widget.h"

NSButton::NSButton
(const char* str, NSButtonListener* bl, void* la, NSButtonCallback* bc, void* ca)
  : NSPlate(up), NSString(str), _listener(bl), _callback(bc),
    buttonPressed(false), cursorOnButton(false), _listenerArg(la), _callbackArg(ca)
{
  gc = XCreateGC(NSdpy, root(), 0, 0);
  XSetFont(NSdpy, gc, font);
  XSetForeground(NSdpy, gc, BlackPixel(NSdpy, 0));
  selectInput(ExposureMask | EnterWindowMask | LeaveWindowMask | 
	      ButtonPressMask | ButtonReleaseMask);
  NSWindow::resize(fontWindowWidth(), fontWindowHeight());
}

void NSButton::resize(unsigned int arg1, unsigned int arg2)
{
  resizable(true);
  fontWindowResize(arg1, arg2);
  NSWindow::resize(arg1, arg2);
  clear();
  XDrawString(NSdpy, window(), gc, hGap(), height() - vGap(), _label.c_str(), _label.length());
  NSPlate::redraw(up);
}

void NSButton::width(unsigned int arg)
{
  resizable(true);
  fontWindowWidth(arg);
  NSWindow::width(arg);
  clear();
  XDrawString(NSdpy, window(), gc, hGap(), height() - vGap(), _label.c_str(), _label.length());
  NSPlate::redraw(up);
}

void NSButton::height(unsigned int arg)
{
  resizable(true);
  fontWindowHeight(arg);
  NSWindow::height(arg);
  clear();
  XDrawString(NSdpy, window(), gc, hGap(), height() - vGap(), _label.c_str(), _label.length());
  NSPlate::redraw(up);
}

void NSButton::label(const char* str)
{
  NSString::label(str);
  NSWindow::resize(fontWindowWidth(), fontWindowHeight());
  clear();
  XDrawString(NSdpy, window(), gc, hGap(), height() - vGap(), _label.c_str(), _label.length());
  NSPlate::redraw(up);
}

void NSButton::redraw()
{
    XDrawString(NSdpy, window(), gc, hGap(), height() - vGap(), _label.c_str(), _label.length());
    NSPlate::redraw(up);  
}

void NSButton::dispatchEvent(const XEvent& ev)
{
  switch (ev.type) {
  case Expose:
    redraw();
    break;
  case EnterNotify:
    cursorOnButton = true;
    if (buttonPressed) mode = down;
    else mode = flat;
    NSPlate::redraw();
    break;
  case LeaveNotify:
    NSPlate::redraw(up);
    cursorOnButton = false;
    break;
  case ButtonPress:
    buttonPressed = true;
    NSPlate::redraw(down);
    break;
  case ButtonRelease:
    buttonPressed = false;
    if (cursorOnButton && _listener != 0) _listener->buttonAction(ev, _listenerArg);
    if (cursorOnButton && _callback != 0) _callback(ev, _callbackArg);
    if (cursorOnButton) mode = flat;
    else mode = up;
    NSPlate::redraw();
    break;
  }
}

// ##### NSToggleButton #####
bool NSToggleButton::initialized = false;
unsigned int NSToggleButton::squareLength;

NSToggleButton::NSToggleButton(const char* str, bool t)
  : NSButton(str)
{
  if (!initialized) {
    initialized = true;
    squareLength = height() - 2 * vGap();
  }

  _toggled = t;
  width(width() + hGap() + squareLength);
}

void NSToggleButton::redraw()
{
  if (_toggled) 
    rect3D(down, width() - hGap() - squareLength, vGap(), squareLength, squareLength);
  else 
    rect3D(up, width() - hGap() - squareLength, vGap(), squareLength, squareLength);
}

void NSToggleButton::dispatchEvent(const XEvent& ev)
{ 
  switch (ev.type) {
  case Expose:
    NSButton::redraw();
    redraw();
    break;
  case EnterNotify:
    NSPlate::redraw(flat);
    break;
  case LeaveNotify:
    NSPlate::redraw(up);
    break;
  case ButtonRelease:
    if (_toggled) _toggled = false; else _toggled = true;
    redraw();
    break;
  }
}
