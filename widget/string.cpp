#include "widget.h"

XFontStruct* NSString::fontStruct;
const int NSString::fontGap;
bool NSString::stringInitialized = false;

NSString::NSString(const char* str)
  : _label(str), _resizable(false)
{
  if (!stringInitialized) {
    stringInitialized = true;

    fontStruct = XQueryFont(NSdpy, font);
  }

  {
    XCharStruct* charStruct = &fontStruct->max_bounds;
    _fontWindowHeight = charStruct->ascent + charStruct->descent + fontGap * 2;
  }

  {
    int direction, ascent, descent;
    XCharStruct charStruct;

    XTextExtents(fontStruct, _label.c_str(), _label.length(), &direction, &ascent, &descent, &charStruct);
  
    _fontWindowWidth = charStruct.rbearing - charStruct.lbearing + fontGap * 2;
    _vGap = (_fontWindowHeight - (charStruct.ascent + charStruct.descent)) / 2;
    _hGap = fontGap;
  }
}

void NSString::label(const char* str)
{
  _label = str;
  
  int direction, ascent, descent;
  XCharStruct charStruct;
  
  XTextExtents(fontStruct, _label.c_str(), _label.length(), &direction, &ascent, &descent, &charStruct);

  if (_resizable) {
    _hGap = (_fontWindowWidth - charStruct.rbearing + charStruct.lbearing) / 2;
    _vGap = (_fontWindowHeight - charStruct.ascent - charStruct.descent) / 2;
  } else {
    _fontWindowWidth = charStruct.rbearing - charStruct.lbearing + fontGap * 2;
    _vGap = (_fontWindowHeight - (charStruct.ascent + charStruct.descent)) / 2;
  }
}

void NSString::fontWindowWidth(unsigned int arg)
{
  int direction, ascent, descent;
  XCharStruct charStruct;
  
  XTextExtents(fontStruct, _label.c_str(), _label.length(), &direction, &ascent, &descent, &charStruct);

  if (_resizable) {
    _fontWindowWidth = arg;
    //_hGap = (_fontWindowWidth - charStruct.rbearing + charStruct.lbearing) / 2;
  }
}

void NSString::fontWindowHeight(unsigned int arg)
{
  int direction, ascent, descent;
  XCharStruct charStruct;
  
  XTextExtents(fontStruct, _label.c_str(), _label.length(), &direction, &ascent, &descent, &charStruct);

  if (_resizable) {
    _fontWindowHeight = arg;
    //_vGap = (_fontWindowHeight - charStruct.ascent - charStruct.descent) / 2;
  }
}
