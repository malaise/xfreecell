#include "widget.h"

class Test : public NSPlate {
public:
  Test() : NSPlate(up) {}

private:
};

int main()
{
  NSInitialize();

  NSFrame frame;
  NSHContainer con(300, 300);
  Test t;

  con.add(&t); con.reallocate();
  frame.container(&con);

  frame.map();

  NSMainLoop();

  return 0;
}
