#include <stdlib.h>
#include "widget.h"

Atom wm_protocols_code = None;
Atom delete_code = None;


unsigned long allocColor(Display* NSdpy, unsigned short red,
                         unsigned short green, unsigned short blue)
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


void NSInitialize(void)
{
  if ((NSdpy = XOpenDisplay(NULL)) == NULL) {
    fprintf(stderr, "Cannot open display\n");
    exit(1);
  }

  font = XLoadFont(NSdpy, defaultFont);
}

static void (*exitCb) (const XEvent &ev);
void NSRegisterExit (const Window toplevel,
                     void (*exitCallback)(const XEvent&))
{
  wm_protocols_code = XInternAtom (NSdpy, "WM_PROTOCOLS", False);
  if (wm_protocols_code != None) {
    delete_code = XInternAtom (NSdpy, "WM_DELETE_WINDOW", False);
  }
  if (XSetWMProtocols (NSdpy, toplevel, &delete_code, 1) == 0 ) {
    fprintf (stderr, "Can't register in WM for deletion\n");
  }
  exitCb = exitCallback;

}

/* Only used in test */
void NSMainLoop() __attribute__ ((noreturn));
void NSMainLoop()
{
  XEvent ev;
  NSWindow* win;
  while (true) {
    NSNextEvent(&ev);
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

  if (ev.type == ClientMessage) {
    if ( ((Atom)ev.xclient.message_type == wm_protocols_code)
      && ((Atom)ev.xclient.data.l[0] == delete_code)) {
      /* Exit requested by WM */
      exitCb (ev);
    }
  }

  win = NSWindow::windowToNSWindow(ev.xany.window);
  if (win != 0) win->dispatchEvent(ev);
}

Display* NSdisplay() { return NSdpy; }

