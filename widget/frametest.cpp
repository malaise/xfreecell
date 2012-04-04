#include "widget.h"
#include <cstdio>

class Test1 : public NSButtonListener {
    void action(const XEvent&, void *);
};

void Test1::action(const XEvent&, void*) { printf("Yes\n"); }

class Test2 : public NSButtonListener {
    void action(const XEvent&, void *);
};

void Test2::action(const XEvent&, void*) { printf("No\n"); }

int main()
{
  NSInitialize();

  NSFrame frame;
  NSHContainer con1(100, 100);
  NSHContainer con2(100, 100);
  NSVContainer con3(100, 200);
  NSButton b1("Yes", new Test1());
  NSButton b2("No", new Test2());
  NSButton b3("Yes", new Test1());
  NSButton b4("No", new Test2());
  b4.resize(50, 50);

  con1.add(&b1); con1.add(&b2);
  con1.reallocate();
  con2.add(&b3); con2.add(&b4);
  con2.reallocate();
  con3.add(&con1); con3.add(&con2);
  con3.reallocate();
  frame.container(&con3);

  frame.map();

  NSMainLoop();

  return 0;
}
