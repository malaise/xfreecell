#include "widget.h"

GC NSTextField::gc;
const unsigned int NSTextField::gap;
bool NSTextField::initialized;
XFontStruct* NSTextField::fontStruct;
unsigned int NSTextField::charWidth, NSTextField::charHeight;

NSTextField::NSTextField(unsigned int arg, NSTextListener* bl, void* la,
                                           NSTextCallback* bc, void* ca)
  : NSPlate(down), _listener(bl), _callback(bc),
    _listenerArg(la), _callbackArg(ca)

{
  if (!initialized) {
    initialized = true;
    fontStruct = XQueryFont(NSdpy, font);
    gc = XCreateGC(NSdpy, window(), 0, 0);
    XSetForeground(NSdpy, gc, bgPixel() ^ BlackPixel(NSdpy, 0));
    XSetFunction(NSdpy, gc, GXxor);
  }

  cursorPos = 0;
  strStart = 0;
  mode = down;
  cursorOnTF = false;
  width(arg);
  height(fontStruct->max_bounds.ascent + fontStruct->max_bounds.descent + gap * 2);
  selectInput(EnterWindowMask | LeaveWindowMask | KeyPressMask | ExposureMask);
  //  charWidth = fontStruct->max_bounds.rbearing - fontStruct->min_bounds.lbearing;
  charWidth = fontStruct->max_bounds.width - 1; // Can't figure out why, but this works.
  charHeight = fontStruct->max_bounds.ascent + fontStruct->max_bounds.descent;
  maxCharNum = width() / charWidth;
}

void NSTextField::draw()
{
  XClearWindow(NSdpy, window());
  NSPlate::redraw();

  int nbToDraw = _str.size() - strStart;
  if (nbToDraw > (int) maxCharNum) nbToDraw = maxCharNum;
  XDrawString(NSdpy, window(), gc, gap, height() - gap, _str.c_str() + strStart, nbToDraw);
  if (cursorOnTF)
    XFillRectangle(NSdpy, window(), gc, gap + (cursorPos - strStart) * charWidth, gap, charWidth, charHeight);
}

void NSTextField::init (const char * str)
{
  _str.clear();
  _str += str;
  if (_str.length() < maxCharNum) strStart = 0;
  else strStart = _str.length() - maxCharNum + 1;
  cursorPos = _str.length();
  draw();

}

void NSTextField::dispatchEvent(const XEvent& ev)
{
  switch (ev.type) {
  case KeyPress:
    {
      const int keyStringLength = 10;
      char keyString[keyStringLength];
      KeySym keysym;
      XKeyEvent xkey = ev.xkey;
      XLookupString(&xkey, keyString, keyStringLength, &keysym, NULL);

      if ((ev.xkey.state & ControlMask) == 0) {
	switch (keysym) {
	case XK_Left:
	  if (cursorPos == strStart && strStart > 0) strStart--;
	  if (cursorPos > 0) cursorPos--; 
	  draw();
	  break;
	case XK_Delete:
	  if (cursorPos < _str.length()) _str.erase(cursorPos, 1);
	  draw();
	  break;
	case XK_Right:
	  if (_str.length() > maxCharNum - 1 && cursorPos == maxCharNum - 1 + strStart) strStart++;
	  if (cursorPos < _str.length()) cursorPos++;
	  draw();
	  break;
	case XK_BackSpace:
	  if (cursorPos == strStart && strStart > 0) strStart--;
	  if (cursorPos > 0) {
	    _str.erase(cursorPos - 1, 1);
	    cursorPos--;
	  }
	  if (strStart > 0) strStart--;
	  draw();
	  break;
	case XK_Linefeed: case XK_Return:
          if (_listener != 0) _listener->textAction (true, _listenerArg);
          if (_callback != 0) _callback(true, _callbackArg);
	  break;
	case XK_Escape:
          if (_listener != 0) _listener->textAction (false, _listenerArg);
          if (_callback != 0) _callback(false, _callbackArg);
	  break;
	}
	if (!isprint(keyString[0])) return;
	cursorPos++;
	if (cursorPos - strStart == maxCharNum) strStart++;
	if (cursorPos == _str.length()) 
	  _str += keyString;
	else 
	  _str.insert(cursorPos - 1, keyString);
	draw();
      } else {
	switch (keysym) {
	case XK_a: case XK_A:
	  cursorPos = 0; strStart = 0; break;
	case XK_b: case XK_B:
	  if (cursorPos == strStart && strStart > 0) strStart--;
	  if (cursorPos > 0) cursorPos--; 
	  break;
	case XK_d: case XK_D:
	  if (cursorPos < _str.length()) _str.erase(cursorPos, 1);
	  break;
	case XK_e: case XK_E:
	  if (_str.length() > maxCharNum - 1) strStart = _str.length() - maxCharNum + 1;
	  cursorPos = _str.length();
	  break;
	case XK_f: case XK_F:
	  if (_str.length() > maxCharNum - 1 && cursorPos == maxCharNum - 1 + strStart) strStart++;
	  if (cursorPos < _str.length()) cursorPos++;
	  break;
	case XK_h: case XK_H:
	  if (cursorPos == strStart && strStart > 0) strStart--;
	  if (cursorPos > 0) {
	    _str.erase(cursorPos - 1, 1);
	    cursorPos--;
	  }
	  if (strStart > 0) strStart--;
	  break;
	case XK_k: case XK_K:
	  _str.erase(cursorPos, _str.length() - cursorPos); break;
	}
	draw();
      }
      break;
    }
  case Expose:
    NSPlate::redraw();
    draw();
    break;
  case EnterNotify:
    XFillRectangle(NSdpy, window(), gc, gap + (cursorPos - strStart) * charWidth, gap, charWidth, charHeight);
    cursorOnTF = true;
    break;
  case LeaveNotify:
    XFillRectangle(NSdpy, window(), gc, gap + (cursorPos - strStart) * charWidth, gap, charWidth, charHeight);
    cursorOnTF = false;
    break;
  }
}
