#include "widget.h"

int main()
{
  NSInitialize();

  NSStaticHContainer con1;
  NSButton b1("Test1");
  NSButton b2("Test2");

  con1.add(&b1); con1.add(&b2);
  con1.reallocate();

  NSStaticHContainer con2;
  NSButton b3("Test3");
  NSButton b4("Test4");

  con2.add(&b3); con2.add(&b4);
  con2.reallocate();

  NSStaticVContainer con3;

  con3.add(&con1); con3.add(&con2);
  con3.reallocate();

  NSFrame frame;

  frame.container(&con3);

  frame.map();

  NSMainLoop();

  return 0;
}
