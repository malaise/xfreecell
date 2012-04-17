#include <vector>

#include "widget.h"

class NSSelector : public NSWindow, public NSScrollbarListener {
public:
  NSSelector(unsigned int width = 100, unsigned int height = 100);
  ~NSSelector();

  const char* selected();

  unsigned int width() const { return NSWindow::width() + NSVScrollbar::defaultWidth(); }
  unsigned int height() const { return NSWindow::height() + NSHScrollbar::defaultHeight(); }

  void x(int arg) { NSWindow::x(arg); _vscroll.x(NSWindow::width() + arg); _hscroll.x(arg); }
  void y(int arg) { NSWindow::y(arg); _vscroll.y(arg); _hscroll.y(NSWindow::height() + arg); }
  void move(int x, int y)
    { NSWindow::move(x, y); _vscroll.move(NSWindow::width() + x, y); _hscroll.move(x, NSWindow::height() + y); }
  void parent(Window arg) { NSWindow::parent(arg); _vscroll.parent(arg); _hscroll.parent(arg); }
  void parent(NSWindow& arg) { parent(arg.window()); }

  void map() { NSWindow::map(); _vscroll.map(); _hscroll.map(); }

  void vScrollAction(const XEvent&, void*, int pos);
  void hScrollAction(const XEvent&, void*, int pos);

  void addItem(const char*);
  void dispatchEvent(const XEvent&);

private:
  void drawBorder(); // I must use this because borderWidth(1) doesn't work.
  void redraw();
  void hilightSelected();

  NSFont _font;

  NSVScrollbar _vscroll;
  NSHScrollbar _hscroll;

  vector<char*> items;
  unsigned int _displayableNum;
  unsigned int _selected;
  bool _hilighted;
  unsigned int _first, _last;

  unsigned int _vGap, _hGap;

  static const unsigned int _defaultVGap, _defaultHGap;
  static bool _initialized;
  static GC _gc;
};

const unsigned int NSSelector::_defaultVGap = 2;
const unsigned int NSSelector::_defaultHGap = 2;
bool NSSelector::_initialized = false;
GC NSSelector::_gc;

NSSelector::NSSelector(unsigned int w, unsigned int h)
  : NSWindow(true),
    _font(font),
    _vscroll(100, h - NSHScrollbar::defaultHeight(), this),
    _hscroll(100, w - NSVScrollbar::defaultWidth(), this)
{
  if (!_initialized) {
    _initialized = true;
    _gc = XCreateGC(NSdpy, window(), 0, 0);
    XSetFunction(NSdpy, _gc, GXxor);
    XSetForeground(NSdpy, _gc, WhitePixel(NSdpy, 0) ^ BlackPixel(NSdpy, 0));
    XSetLineAttributes(NSdpy, _gc, 1, LineSolid, CapButt, JoinMiter);
  }

  _selected = 0;
  _hilighted = false;
  _first = _last = 0;

  _vGap = _defaultVGap;
  _hGap = _defaultHGap;

  resize(w - NSVScrollbar::defaultWidth(), h - NSHScrollbar::defaultHeight());
  borderWidth(1);
  border(nameToPixel("black"));
  XFlush(NSdpy);
  background(nameToPixel("white"));
  selectInput(ButtonPressMask | ExposureMask);

  _displayableNum = (NSWindow::height() - _vGap) / (_font.height() + _vGap);
}

NSSelector::~NSSelector()
{
  for (unsigned int i = 0; i < items.size(); i++)
    delete items[i];
}

const char* NSSelector::selected()
{
  if (_first <= _selected && _selected < _last)
    return items[_selected];

  return 0;
}

void NSSelector::vScrollAction(const XEvent&, void*, int delta)
{
  _first += delta;
  if (_first < 0) _first = 0;
  if (_first >= items.size()) _first = items.size() - 1;

  _last += delta;
  if (_last < 0) _last = 0;
  if (_last > items.size()) _last = items.size();

  redraw();
}

void NSSelector::hScrollAction(const XEvent&, void*, int delta)
{
  _hGap -= delta;
  redraw();
}

void NSSelector::addItem(const char* str)
{
  char* newStr = new char[strlen(str) + 1];
  strcpy(newStr, str);
  items.push_back(newStr);

  if (_last - _first + 1 <= _displayableNum) {
    _last = items.size();
    XDrawString(NSdpy, window(), _gc, _hGap,
                (_vGap + _font.height()) * (_last - _first),
                items[_last - 1], strlen(items[_last - 1]));
  } else {
    _vscroll.barPercent(_displayableNum * 100 / items.size());
    _vscroll.movement(_vscroll.height() / items.size());
  }

  unsigned int strWidth = _font.textWidth(str);
  unsigned int width = NSWindow::width();

  if (strWidth > width)
    _hscroll.barPercent(width * 100 / strWidth);
}

inline void NSSelector::drawBorder()
{
  XPoint points[] = { {0, 0}, {NSWindow::width() - 1, 0},
                      {NSWindow::width() - 1, NSWindow::height() - 1},
                      {0, NSWindow::height() - 1}, {0, 0} };

  XDrawLines(NSdpy, window(), _gc, points, 5, CoordModeOrigin);
}

void NSSelector::redraw()
{
  if (_first == _last) return;

  XClearWindow(NSdpy, window());
  for (unsigned int i = _first; i < _last; i++) {
    XDrawString(NSdpy, window(), _gc, _hGap, (_vGap + _font.height()) * (i - _first + 1),
                items[i], strlen(items[i]));
  }
  if (_hilighted)
    hilightSelected();
  drawBorder(); // I must use this because borderWidth(1) doesn't work.
}

void NSSelector::dispatchEvent(const XEvent& ev)
{
  switch (ev.type) {
  case Expose:
    if (ev.xexpose.count == 0)
      redraw();
    break;
  case ButtonPress:
    unsigned int tmp;
    tmp = ev.xbutton.y / (_font.height() + _vGap) + _first;
    if (_selected == tmp) {
      _selected = tmp;
      hilightSelected();
      if (_hilighted) _hilighted = false;
      else _hilighted = true;
    } else {
      if (_hilighted)
        hilightSelected();
      _selected = tmp;
      hilightSelected();
      _hilighted = true;
    }
    break;
  }
}

inline void NSSelector::hilightSelected()
{
  if (_first <= _selected && _selected < _last) {
    XFillRectangle(NSdpy, window(), _gc, _hGap,
                   (_vGap + _font.height()) * (_selected - _first) + _vGap,
                   _font.textWidth(items[_selected]), _font.height());
  }
}

int main()
{
  NSInitialize();

  NSFrame frame;
  NSHContainer con(300, 300);
  NSSelector nss;

  nss.addItem("This");
  nss.addItem("is");
  nss.addItem("a");
  nss.addItem("test.");
  nss.addItem("TEST");
  nss.addItem("abc");
  nss.addItem("def");
  nss.addItem("ghi");
  nss.addItem("Long string to test\n");

  for (int i = 0; i < 20; i++)
    nss.addItem("abc");
  nss.addItem("end");

  con.add(&nss); con.reallocate();
  frame.container(&con);

  frame.map();
  nss.map();

  XFlush(NSdisplay());

  NSMainLoop();

  return 0;
}
