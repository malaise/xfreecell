#include <stdlib.h>
#include "widget.h"

unsigned long allocColor(Display* NSdpy, unsigned int red, unsigned int green, unsigned int blue)
{
  XColor col = { 0, red, green, blue, 0, 0};
  if (XAllocColor(NSdpy, DefaultColormap(NSdpy, 0), &col) == 0) {
    printf(" Warning : Color map full (%x,%x,%x) \n",red,green,blue); 
    return WhitePixel(NSdpy, 0);
  }
  return col.pixel;
}

unsigned long nameToPixel(const char* name)
{
  Colormap cmap;
  XColor c0, c1;

  cmap = DefaultColormap(NSdpy, 0);
  XAllocNamedColor(NSdpy, cmap, name, &c1, &c0);

  return c1.pixel;
}

void NSInitialize()
{
  if ((NSdpy = XOpenDisplay(NULL)) == NULL) {
    fprintf(stderr, "Cannot open display\n");
    exit(1);
  }
  font = XLoadFont(NSdpy, defaultFont);
}
 
void NSMainLoop() __attribute__ ((noreturn));
void NSMainLoop()
{
  XEvent ev;
  NSWindow* win;
  while (true) {
    XNextEvent(NSdpy, &ev);
    win = NSWindow::windowToNSWindow(ev.xany.window);
    if (win != 0) win->dispatchEvent(ev);
  }
}

void NSNextEvent(XEvent* ev)
{
  XNextEvent(NSdpy, ev);
}

void NSDispatchEvent(const XEvent& ev)
{
  NSWindow* win;

  win = NSWindow::windowToNSWindow(ev.xany.window);
  if (win != 0) win->dispatchEvent(ev);
}

Display* NSdisplay() { return NSdpy; }
