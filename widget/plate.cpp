#include "widget.h"

bool NSPlate::plateInitialized = false;
GC NSPlate::bgGC, NSPlate::bRimGC, NSPlate::dRimGC;
unsigned long NSPlate::_bgPixel, NSPlate::_bRimPixel, NSPlate::_dRimPixel;
const int NSPlate::_thickness;

NSPlate::NSPlate(Mode m)
  : NSWindow(true, RootWindow(NSdpy, 0)), mode(m)
{
  if (!plateInitialized) {
    plateInitialized = true;

    _bgPixel = allocColor(NSdpy, 0x6180,0xa290,0xc300);
    _bRimPixel = allocColor(NSdpy, 0x8a20,0xe380,0xffff);
    _dRimPixel = allocColor(NSdpy, 0x30c0,0x5144,0x6180);

    bgGC = XCreateGC(NSdpy, root(), 0, 0);
    XSetForeground(NSdpy, bgGC, _bgPixel);
    bRimGC = XCreateGC(NSdpy, root(), 0, 0);
    XSetForeground(NSdpy, bRimGC, _bRimPixel);
    dRimGC = XCreateGC(NSdpy, root(), 0, 0);
    XSetForeground(NSdpy, dRimGC, _dRimPixel);
  }

  selectInput(ExposureMask);
  background(_bgPixel);
}

void NSPlate::rect3D(Mode m, int x, int y, unsigned int w, unsigned int h)
{
  GC upperLeft, bottomRight;
  switch (m) {
  case up: upperLeft = bRimGC; bottomRight = dRimGC; break;
  case down: upperLeft = dRimGC; bottomRight = bRimGC; break;
  case flat: default: upperLeft = bottomRight = bgGC; break;
  }

  for (int i = 0; i < _thickness; i++) {
    short xa = x + i, ya = y + i, xe = x + w - i, ye = y + h - i;
    XPoint xp[5] = { {xa,ye}, {xa,ya}, {xe,ya}, {xe,ye}, {xa,ye}};
    XDrawLines(NSdpy, window(), upperLeft, xp, 3, CoordModeOrigin);
    XDrawLines(NSdpy, window(), bottomRight, xp+2, 3, CoordModeOrigin);
  }
}

void NSPlate::dispatchEvent(const XEvent& ev)
{
  if (ev.type == Expose) redraw();
}
