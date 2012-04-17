#include "widget.h"

GC NSLabel::gc;
bool NSLabel::initialized = false;

NSLabel::NSLabel(const char* str)
  : NSWindow(true), NSString(str)
{
  if (!initialized) {
    gc = XCreateGC(NSdpy, root(), 0, 0);
    XSetFont(NSdpy, gc, font);
    XSetForeground(NSdpy, gc, BlackPixel(NSdpy, 0));
    initialized = true;
  }
  selectInput(ExposureMask);
  NSWindow::resize(fontWindowWidth(), fontWindowHeight());
  NSWindow::background(WhitePixel(NSdpy, 0));
}

void NSLabel::label(const char* str)
{
    NSString::label(str);
    resize(fontWindowWidth(), fontWindowHeight());
    XClearWindow(NSdpy, window());
    XDrawString(NSdpy, window(), gc, hGap(), height() - vGap(), _label.c_str(), _label.length());
}

void NSLabel::dispatchEvent(const XEvent& ev)
{
  if (ev.type == Expose && ev.xexpose.count == 0)
    XDrawString(NSdpy, window(), gc, hGap(), height() - vGap(), _label.c_str(), _label.length());
}
