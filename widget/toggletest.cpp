#include "widget.h"

int main()
{
  NSInitialize();

  NSHContainer con(100, 100);
  NSFrame frame;
  NSToggleButton b("test");
  //  NSButton b("test");

  con.add(&b);
  frame.container(&con);
  frame.map();

  NSMainLoop();

  return 0;
}
