#include "widget.h"

int main()
{
  NSInitialize();

  NSTextField tf(100);

  tf.map();

  NSMainLoop();

  return 0;
}
