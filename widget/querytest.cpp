#include "widget.h"
#include <cstdio>

class Yes : public NSButtonListener {
    void action(const XEvent&, void *);
};

void Yes::action(const XEvent&, void*) { printf("Yes\n"); }

class No : public NSButtonListener {
    void action(const XEvent&, void *);
};

void No::action(const XEvent&, void*) { printf("No\n"); }

int main()
{
  NSInitialize();

  NSFrame frame;
  NSButton yes("Yes", new Yes());
  NSButton no("No", new No());
  NSLabel label("Yes or No");
  NSVContainer con1(100, 200);
  NSHContainer con2(100, 100);

  frame.resize(100, 200);
  con2.add(&yes);
  con2.add(&no);
  con2.reallocate();
  con1.add(&label);
  con1.add(&con2);
  con1.reallocate();
  frame.container(&con1);

  frame.map();

  NSMainLoop();

  return 0;
}
