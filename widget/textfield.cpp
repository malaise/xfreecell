#include "widget.h"

/* Specific values of select_index, other (natural) values
 *   are indexes in selection_types */
#define SELEC_NONE -1
#define SELEC_STORED 1000  /* Stored index + SELEC_STORED */


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
  selectInput(EnterWindowMask | LeaveWindowMask | KeyPressMask | ExposureMask | ButtonPressMask);
  //  charWidth = fontStruct->max_bounds.rbearing - fontStruct->min_bounds.lbearing;
  charWidth = fontStruct->max_bounds.width - 1; // Can't figure out why, but this works.
  charHeight = fontStruct->max_bounds.ascent + fontStruct->max_bounds.descent;
  maxCharNum = width() / charWidth;
  selectIndex = SELEC_NONE;

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

void NSTextField::requestSelection (void) {
  (void) XConvertSelection (NSdpy, XA_PRIMARY,
        selectionTypes[selectIndex], selectCode, window(), CurrentTime);

}

void NSTextField::dispatchEvent(const XEvent& ev)
{
  switch (ev.type) {
    case KeyPress: {
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
            cursorPos = 0; strStart = 0;
            break;
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
            _str.erase(cursorPos, _str.length() - cursorPos);
            break;
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
    case ButtonPress:
      if (ev.xbutton.button == 1) {
        cursorPos = ev.xbutton.x / charWidth;
        if (_str.length() <= maxCharNum - 1 && cursorPos > _str.length()) cursorPos = _str.length();
        draw();
      } else if (ev.xbutton.button == 2) {
        /* Move cursor to pointer position */
        /* Get position */
        /* Convert to char index */
        /* If above last char then last char */
      } else if (ev.xbutton.button == 2) {
        /* Request selection */
        selectIndex = 0;
        this->requestSelection ();
      }
      break;
    case SelectionNotify:
      if (ev.xselection.property == None) {
        /* Selection transfer failed  for this target type */
        selectIndex++;
        if (selectIndex < NB_SELECTION_TYPES) {
          /* Try next target ype */
          this->requestSelection ();
        } else {
          /* No more supported target types => failed */
          selectIndex = SELEC_NONE;
        }
      } else {
        /* Success */
        int res, i;
        Atom typeReturn;
        int formatReturn;
        unsigned long nitemsReturn, offsetReturn;
        char *data;
        char strTmp[2];
        /* Get the selection and delete it */
        res = XGetWindowProperty (NSdpy, window(), selectCode,
          0L, (long)maxCharNum, True, selectIndex,
          &typeReturn, &formatReturn, &nitemsReturn, &offsetReturn,
          (unsigned char **)&data);
        XDeleteProperty (NSdpy, window(), selectCode);
        selectIndex = SELEC_NONE;
        /* Append selection */
        if (res != Success) return;
        for (i = 0; i < nitemsReturn; i++) {
          if (!isprint(data[i])) return;
          strTmp[0] = data[i];
          strTmp[1] = 0;
          cursorPos++;
          if (cursorPos - strStart == maxCharNum) strStart++;
          if (cursorPos == _str.length()) {
            _str += strTmp;
          } else {
            _str.insert(cursorPos - 1, strTmp);
          }
        }
        draw();
      }

      break;

  }
}

