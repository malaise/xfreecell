#include "widget.h"

NSFrame::NSFrame(NSContainer* nscArg, int x, int y, unsigned int width, unsigned int height,
                 unsigned int borderWidth, unsigned long border)
  : NSWindow(true, RootWindow(NSdpy, 0), x, y, width, height, borderWidth,
             border, WhitePixel(NSdpy, 0)),
  _container(nscArg)
{
  if (_container != 0) {
    _container->move(0, 0);
    _container->parent(window());
    NSWindow::set_width(_container->width());
    NSWindow::set_height(_container->height());
    _container->parent(NSWindow::window());
  }
}

void NSFrame::container(NSContainer* nsc)
{
  _container = nsc;
  _container->parent(NSWindow::window());
  NSWindow::set_width(nsc->width());
  NSWindow::set_height(nsc->height());
}

