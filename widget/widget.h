#ifndef Widget_H
#define Widget_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <vector>
#include <string>

const char defaultFont[] = "7x14";

extern Display* NSdpy;
extern Font font;

class NSComponent;
class NSWindow;
class NSPanel;
class NSString;
class NSLabel;
class NSButtonListener;
class NSButton;
class NSToggleButton;
class NSTextListener;
class NSTextField;
class NSContainer;
class NSHContainer;
class NSVContainer;
class NSStaticHContainer;
class NSStaticVContainer;
class NSFrame;
class NSScrollbarListener;
class NSVScrollbar;

unsigned long allocColor(Display*, unsigned short R, unsigned short G,
                                   unsigned short B);
unsigned long nameToPixel(const char* colorName);
void NSInitialize();
void NSRegisterExit(const Window toplevel,
                    void (*exitCallback)(const XEvent&));
void NSMainLoop();
void NSNextEvent(XEvent*);
void NSDispatchEvent(const XEvent&);
Display* NSdisplay();

/* Number of supported types of XA_PRIMARY clipborad exchanges */
#define NB_SELECTION_TYPES 3

/* Selection types and selection code */
extern Atom selectionTypes[NB_SELECTION_TYPES];
extern Atom selectCode;


// ##### NSComponent #####
class NSComponent {
public:
  virtual ~NSComponent() {}
  virtual int x() const = 0;
  virtual int y() const = 0;
  virtual unsigned int width() const = 0;
  virtual unsigned int height() const = 0;

  virtual void move(int, int) = 0;
  virtual void x(int) = 0;
  virtual void y(int) = 0;
  virtual void parent(NSWindow&) = 0;
  virtual void parent(Window) = 0;

  virtual void map() = 0;
  virtual void unmap() = 0;
};

// ##### NSWindow #####
class NSWindow : public NSComponent {
public:
  NSWindow(bool create, Window parent = RootWindow(NSdpy, 0), int x = 0,
           int y = 0, unsigned int width = 50, unsigned int height = 50,
           unsigned int borderWidth = 0, unsigned long border = 0,
           unsigned long bg = 0);
  virtual ~NSWindow();

  void window(Window w) { _window = w; }
  void move(int, int);
  void x(int arg) { move(arg, _y); }
  void y(int arg) { move(_x, arg); }
  void resize(unsigned int, unsigned int);
  void width(unsigned int arg) { resize(arg, _height); }
  void height(unsigned int arg) { resize(_width, arg); }
  void borderWidth(unsigned int arg) const { XSetWindowBorderWidth(NSdpy, _window, arg); }
  void border(unsigned long arg) const { XSetWindowBorder(NSdpy, _window, arg); }
  void parent(Window arg) { XReparentWindow(NSdpy, _window, arg, _x, _y); }
  void parent(NSWindow& arg) { XReparentWindow(NSdpy, _window, arg.window(), _x, _y); }
  void background(unsigned long pixel) const { XSetWindowBackground(NSdpy, _window, pixel); }
  void backgroundPixmap(Pixmap p) const { XSetWindowBackgroundPixmap(NSdpy, _window, p); }
  void selectInput(long);
  void map();
  void unmap();
  bool mapped();
  void setMaxMinSize(unsigned int minW, unsigned int minH, unsigned int maxW, unsigned int maxH);
  void raise() const { XRaiseWindow(NSdpy, _window); }
  void clear() const { XClearWindow(NSdpy, _window); }

  Window window() { return _window; }
  int x() const { return _x; }
  int y() const { return _y; }
  unsigned int height() const { return _height; }
  unsigned int width() const { return _width; }
  Window root() const { return _root; }

  virtual void dispatchEvent(const XEvent&) {}

  static NSWindow* windowToNSWindow(Window);

private:
  Window _window;
  int _x, _y;
  unsigned int _height, _width;
  bool _mapped;

  static bool windowInitialized;
  static Window _root;

  struct Elt {
    NSWindow* nswindow;
    Window window;

#ifdef GCC272
    Elt() {}
#endif
    Elt(NSWindow* nw, Window w) { nswindow = nw; window = w; }
  };

  static std::vector<Elt> eltVector;
  static void eraseWindow(Window);
  static void registerWindow(NSWindow*, Window);
};

// ##### NSPlate #####
class NSPlate : public NSWindow {
protected:
  enum Mode { up, down, flat };
public:
  NSPlate(Mode);
  virtual void dispatchEvent(const XEvent&);

protected:
  Mode mode;
  void rect3D(Mode, int x, int y, unsigned int width, unsigned int height);
  virtual void redraw() { rect3D(mode, 0, 0, width(), height()); }
  void redraw(Mode m) { mode = m; rect3D(mode, 0, 0, width(), height()); }

  unsigned long bgPixel() const { return _bgPixel; }
  unsigned long bRimPixel() const { return _bRimPixel; }
  unsigned long dRimPixel() const { return _dRimPixel; }

  int thickness() const { return _thickness; }

private:
  static bool plateInitialized;

  static GC bgGC, bRimGC, dRimGC;
  static unsigned long _bgPixel, _bRimPixel, _dRimPixel;
  static const int _thickness = 2;
};

// ##### NSString #####

class NSString {
public:
  NSString(const char*);
  virtual ~NSString() {}

  const char* label() const { return _label.c_str(); }
  unsigned int length() const { return _label.length(); }

  virtual void label(const char*);

protected:
  unsigned int fontWindowWidth() const { return _fontWindowWidth; }
  unsigned int fontWindowHeight() const { return _fontWindowHeight; }
  int vGap() const { return _vGap; }
  int hGap() const { return _hGap; }
  bool resizable() const { return _resizable; }

  void fontWindowResize(unsigned int arg1, unsigned int arg2) { fontWindowWidth(arg1); fontWindowHeight(arg2); }
  void fontWindowWidth(unsigned int);
  void fontWindowHeight(unsigned int);
  void resizable(bool arg) { _resizable = arg; }

  std::string _label;
private:
  static const int fontGap = 4;
  static XFontStruct* fontStruct;
  unsigned int _fontWindowHeight;
  unsigned int _fontWindowWidth;
  int _vGap, _hGap;

  bool _resizable;

  static bool stringInitialized;
};

// ##### NSLabel #####
class NSLabel : public NSWindow, public NSString {
public:
  NSLabel(const char*);

  void label(const char*);
  virtual void dispatchEvent(const XEvent&);

private:
  static GC gc;
  static bool initialized;
};

// ##### NSButton #####
class NSButtonListener {
public:
  virtual ~NSButtonListener() {}

  virtual void buttonAction(const XEvent&, void *) = 0;
};

typedef void NSButtonCallback(const XEvent&, void*);

class NSButton : public NSPlate, public NSString {
public:
  NSButton(const char*, NSButtonListener* = 0, void* = 0, NSButtonCallback* = 0, void* = 0);

  unsigned int width() { return NSWindow::width(); }
  unsigned int height() { return NSWindow::height(); }

  void resize(unsigned int, unsigned int);
  void width(unsigned int);
  void height(unsigned int);
  void label(const char*);
  //  void listener(NSButtonListener* bl) { _listener = bl; }
  //  void listenerArg(void* la) { _listener = la; }
  //  void callback(NSButtonCallback* bc) { _callback = bc; }
  //  void callbackArg(void* ba) { _callbackArg = ba; }

  void dispatchEvent(const XEvent&);
  virtual void redraw();

private:
  GC gc;
  NSButtonListener* const _listener;
  NSButtonCallback* const _callback;
  bool buttonPressed;
  bool cursorOnButton;
  void* const _listenerArg;
  void* const _callbackArg;
};

// ##### NSToggleButton #####
class NSToggleButton : public NSButton {
private:
  static bool initialized;
  static unsigned int squareLength;
  bool _toggled;

public:
  NSToggleButton(const char*, bool = false);

  bool toggled() { return _toggled; }
  void redraw();

  void toggled(bool arg) { _toggled = arg; }

  void dispatchEvent(const XEvent&);
};

// ##### NSTextField #####
class NSTextListener {
public:
  virtual ~NSTextListener() {}

  virtual void textAction(bool, void *) = 0;
};

typedef void NSTextCallback(bool, void*);

class NSTextField : public NSPlate {
public:
  NSTextField(unsigned int width, NSTextListener* = 0, void* = 0, NSTextCallback* = 0, void* = 0);

  const char* text() const { return _str.c_str(); }
  void clear () { _str.clear(); cursorPos = 0; strStart = 0; }
  void init (const char * str);
  void dispatchEvent(const XEvent&);
  void requestSelection(void);
private:
  void draw();

  static GC gc;
  static const unsigned int gap = 4;
  static bool initialized;
  static XFontStruct* fontStruct;
  static unsigned int charWidth, charHeight;
  NSTextListener* const _listener;
  NSTextCallback* const _callback;
  void* const _listenerArg;
  void* const _callbackArg;

  unsigned int maxCharNum;
  std::string _str;
  unsigned int cursorPos;
  unsigned int strStart;
  bool cursorOnTF;
  int selectIndex;
};

// ##### NSContainer #####
class NSContainer : public NSComponent {
public:
  NSContainer() : _width(0), _height(0), _x(0), _y(0), _vGap(0), _hGap(0), _neighborGap(0), _parentWindow(0) {}
  NSContainer(unsigned int vGap, unsigned int hGap, unsigned int neighborGap)
    : _width(0), _height(0), _x(0), _y(0), _vGap(vGap), _hGap(hGap),
      _neighborGap(neighborGap), _parentWindow(0) {}
  virtual ~NSContainer() {}

  int x() const { return _x; }
  int y() const { return _y; }
  unsigned int width() const { return _width; }
  unsigned int height() const { return _height; }
  unsigned int size() const { return nscVec.size(); }
  unsigned int vGap() { return _vGap; }
  unsigned int hGap() { return _hGap; }
  unsigned int neighborGap() { return _neighborGap; }

  void move(int, int);
  void x(int);
  void y(int);
  void parent(Window);
  void parent(NSWindow&);
  void vGap(unsigned int arg) { _vGap = arg; reallocate(); }
  void hGap(unsigned int arg) { _hGap = arg; reallocate(); }
  void neighborGap(unsigned int arg) { _neighborGap = arg; reallocate(); }

  virtual void add(NSComponent*);
  virtual void remove(NSComponent*);
  virtual void reallocate() = 0;

  void map();
  void unmap();

protected:
  void hremove(NSComponent*);
  void vremove(NSComponent*);
  void hreallocate();
  void vreallocate();
  void vsreallocate();
  void hsreallocate();

  unsigned int _width, _height;

private:
  int _x, _y;
  unsigned int _vGap, _hGap, _neighborGap;
  Window _parentWindow;

  std::vector<NSComponent*> nscVec;
};

// ##### NSHContainer #####
class NSHContainer : public NSContainer {
public:
  NSHContainer() : NSContainer() {}
  NSHContainer(unsigned int w, unsigned int h) : NSContainer() { resize(w, h); }

  void add(NSComponent* arg) { NSContainer::add(arg); }
  void remove(NSComponent* arg) { NSContainer::remove(arg); }
  void resize(unsigned int arg1, unsigned int arg2) { width(arg1); height(arg2); }
  void width(unsigned int arg) { _width = arg; }
  void height(unsigned int arg) { _height = arg; }
  void reallocate() { NSContainer::hreallocate(); }
};

// ##### NSVContainer #####
class NSVContainer : public NSContainer {
public:
  NSVContainer() : NSContainer() {}
  NSVContainer(unsigned int w, unsigned int h) : NSContainer() { resize(w, h); }

  void add(NSComponent* arg) { NSContainer::add(arg); }
  void remove(NSComponent* arg) { NSContainer::remove(arg); }
  void resize(unsigned int arg1, unsigned int arg2) { width(arg1); height(arg2); }
  void width(unsigned int arg) { _width = arg; }
  void height(unsigned int arg) { _height = arg; }
  void reallocate() { NSContainer::vreallocate(); }
};

// ##### NSStaticHContainer #####
class NSStaticHContainer : public NSContainer {
public:
  NSStaticHContainer(unsigned int hGap = 0, unsigned int vGap = 0, unsigned int neighborGap = 0)
    : NSContainer(hGap, vGap, neighborGap) {}

  void add(NSComponent*);
  void remove(NSComponent*);
  void reallocate() { NSContainer::hsreallocate(); }
};

// ##### NSStaticVContainer #####
class NSStaticVContainer : public NSContainer {
public:
  NSStaticVContainer(unsigned int hGap = 0, unsigned int vGap = 0,
                     unsigned int neighborGap = 0)
    : NSContainer(hGap, vGap, neighborGap) {}

  void add(NSComponent*);
  void remove(NSComponent*);
  void reallocate() { NSContainer::vsreallocate(); }
};

// ##### NSFrame #####
class NSFrame : public NSWindow {
public:
    NSFrame(NSContainer* = 0, int x = 0, int y = 0, unsigned int width = 50,
            unsigned int height = 50, unsigned int borderWidth = 0,
            unsigned long border = 0);

    void container(NSContainer*);
    void map() { if (_container != 0) _container->map(); NSWindow::map(); }
private:
    NSContainer* _container;
};

// ##### NSScrollbar #####
class NSScrollbarListener {
public:
  virtual ~NSScrollbarListener() {}

  virtual void vScrollAction(const XEvent&, void*, int pos) = 0;
  virtual void hScrollAction(const XEvent&, void*, int pos) = 0;
};

// ## NSVScrollbar ##
class NSVScrollbar : public NSPlate {
public:
  NSVScrollbar(unsigned int barHeight, unsigned int height, NSScrollbarListener* nsl = 0, void* nslArg = 0);

  unsigned int width() const { return NSWindow::width(); }
  unsigned int height() const { return NSWindow::height(); }
  unsigned int currentPos() const; // 1 <= currentPos <= height() - thickness * 2 - barPercent
  unsigned int barHeight() const { return _bar.height(); }
  static unsigned int defaultWidth() { return _defaultWidth; }

  void width(unsigned int) {}
  void height(unsigned int arg) { NSWindow::height(arg); }
  void resize(unsigned int, unsigned int arg) { height(arg); }
  void barPercent(unsigned int arg) { barHeight((height() - thickness() * 2) * arg / 100); }
  void barHeight(unsigned int arg);
  void movement(unsigned int arg) { _movement = arg; }

  void map() { NSPlate::map(); _bar.map(); }
  void dispatchEvent(const XEvent&);

private:
  bool outOfBar(int);
  bool insideBar(int y) { return !outOfBar(y); }
  bool inLimits(int);
  int appropriatePos(int);

  NSPlate _bar;
  bool _buttonPressed;

  unsigned int _movement;

  NSScrollbarListener* const listener;
  void* const listenerArg;

  static const unsigned int _defaultWidth;
};

// ## NSHScrollbar ##
class NSHScrollbar : public NSPlate {
public:
  NSHScrollbar(unsigned int barWidth, unsigned int width, NSScrollbarListener* nsl = 0, void* nslArg = 0);

  unsigned int width() const { return NSWindow::width(); }
  unsigned int height() const { return NSWindow::height(); }
  unsigned int currentPos() const; // 1 <= currentPos <= height() - thickness * 2 - barPercent
  unsigned int barWidth() const { return _bar.width(); }
  static unsigned int defaultHeight() { return _defaultHeight; }

  void width(unsigned int arg) { NSWindow::width(arg); }
  void height(unsigned int) {}
  void resize(unsigned int arg, unsigned int) { width(arg); }
  void barPercent(unsigned int arg) { barWidth((width() - thickness() * 2) * arg / 100); }
  void barWidth(unsigned int);
  void movement(unsigned int arg) { _movement = arg; }

  void map() { NSPlate::map(); _bar.map(); }
  void dispatchEvent(const XEvent&);

private:
  bool outOfBar(int x);
  bool insideBar(int x) { return !outOfBar(x); }
  bool inLimits(int x);
  int appropriatePos(int);

  NSPlate _bar;
  bool _buttonPressed;

  unsigned int _movement;

  NSScrollbarListener* const listener;
  void* const listenerArg;

  static const unsigned int _defaultHeight;
};

// ##### NSFont #####
class NSFont {
public:
  NSFont();
  NSFont(Font);

  unsigned int height() { return _fontStruct->max_bounds.ascent + _fontStruct->max_bounds.descent; }
  unsigned int width() { return _fontStruct->max_bounds.rbearing - _fontStruct->max_bounds.lbearing; }

  void textExtents(const char* str, XCharStruct*, int* dir = 0, int* ascent = 0, int* descent = 0);
  unsigned int textWidth(const char*);
  unsigned int textHeight(const char*);

private:
  XFontStruct* _fontStruct;
  Font _font;
};
#endif
