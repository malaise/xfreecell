#include <string.h>
#include "widget.h"

NSFont::NSFont()
{
  _font = XLoadFont(NSdpy, defaultFont);
  _fontStruct = XQueryFont(NSdpy, _font);
}

NSFont::NSFont(Font f)
{
  _font = f;
  _fontStruct = XQueryFont(NSdpy, _font);
}

void NSFont::textExtents(const char* str, XCharStruct* cs, int* dir, int* asc, int* desc)
{
  int direction, ascent, descent;

  XTextExtents(_fontStruct, str, strlen(str), &direction, &ascent, &descent, cs);

  if (dir != 0) *dir = direction;
  if (asc != 0) *asc = ascent;
  if (desc != 0) *desc = descent;
}

unsigned int NSFont::textWidth(const char* str)
{
  XCharStruct cs;

  textExtents(str, &cs);
  return cs.rbearing - cs.lbearing;
}

unsigned int NSFont::textHeight(const char* str)
{
  XCharStruct cs;

  textExtents(str, &cs);
  return cs.ascent + cs.descent;
}
