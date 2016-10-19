#include "widget.h"

void NSContainer::move(int arg1, int arg2)
{
  int deltaX = arg1 - _x;
  int deltaY = arg2 - _y;
  for (unsigned int i = 0; i < nscVec.size(); i++)
    nscVec[i]->move(nscVec[i]->x() + deltaX, nscVec[i]->y() + deltaY);

  _x = arg1; _y = arg2;
}

void NSContainer::x(int arg)
{
  int delta = arg - _x;
  for (unsigned int i = 0; i < nscVec.size(); i++)
    nscVec[i]->x(nscVec[i]->x() + delta);
  _x = arg;
}

void NSContainer::y(int arg)
{
  int delta = arg - _y;
  for (unsigned int i = 0; i < nscVec.size(); i++)
    nscVec[i]->y(nscVec[i]->y() + delta);
  _y = arg;
}

void NSContainer::parent(Window w)
{
  _parentWindow = w;
  for (unsigned int i = 0; i < nscVec.size(); i++)
    nscVec[i]->parent(w);
}

void NSContainer::parent(NSWindow& nsw)
{
  _parentWindow = nsw.window();
  for (unsigned int i = 0; i < nscVec.size(); i++)
    nscVec[i]->parent(nsw);
}

void NSContainer::add(NSComponent* nsc)
{
  nscVec.push_back(nsc);
  if (_parentWindow != 0)
      nsc->parent(_parentWindow);
}

void NSContainer::map()
{
  for (unsigned int i = 0; i < nscVec.size(); i++)
    nscVec[i]->map();
}

void NSContainer::unmap()
{
  for (unsigned int i = 0; i < nscVec.size(); i++)
    nscVec[i]->unmap();
}

void NSContainer::remove(NSComponent* nsc)
{
  std::vector<NSComponent*>::iterator iter;
  for (iter = nscVec.begin(); iter != nscVec.end(); iter++)
    if (*iter == nsc) nscVec.erase(iter);
}

void NSContainer::hremove(NSComponent* nsc)
{
  std::vector<NSComponent*>::iterator iter;
  for (iter = nscVec.begin(); iter != nscVec.end(); iter++) {
    if (*iter == nsc) {
      int w = (*iter)->width() + _neighborGap;
      std::vector<NSComponent*>::iterator iter2 = iter + 1;
      if (nscVec.size() == 0) {
        _width = 0; _height = 0;
      } else {
        for (; iter2 != nscVec.end(); iter2++)
          (*iter2)->x((*iter2)->x() - w);
        _width -= w;
      }
      nscVec.erase(iter);
    }
  }
}

void NSContainer::vremove(NSComponent* nsc)
{
  std::vector<NSComponent*>::iterator iter;
  for (iter = nscVec.begin(); iter != nscVec.end(); iter++) {
    if (*iter == nsc) {
      int h = (*iter)->height() + _neighborGap;
      std::vector<NSComponent*>::iterator iter2 = iter + 1;
      if (nscVec.size() == 0) {
        _width = 0; _height = 0;
      } else {
        for (; iter2 != nscVec.end(); iter2++)
          (*iter2)->y((*iter2)->y() - h);
        _height -= h;
      }
      nscVec.erase(iter);
    }
  }
}

void NSContainer::hreallocate()
{
  unsigned int gap = _width / (nscVec.size() + 1);

  unsigned int g = gap, i = 0;
  for (; i < nscVec.size(); g += gap, i++) {
    int a = x() + g - nscVec[i]->width() / 2;
    int b = y() + (_height - nscVec[i]->height()) / 2;
    nscVec[i]->move(a, b);
  }
}

void NSContainer::vreallocate()
{
  unsigned int gap = _height / (nscVec.size() + 1);

  unsigned int g = gap, i = 0;
  for (; i < nscVec.size(); g += gap, i++) {
    int a = x() + (_width - nscVec[i]->width()) / 2;
    int b = y() + g - nscVec[i]->height() / 2;
    nscVec[i]->move(a, b);
  }
}

void NSContainer::hsreallocate()
{
    if (nscVec.size() == 0) {
      _width = _height = 0;
      return;
    }

    _width = _hGap;
    for (unsigned int i = 0; i < nscVec.size(); i++) {
      nscVec[i]->move(x() + _width, y() + _vGap);
      _width = _width + nscVec[i]->width() + _neighborGap;
      if (nscVec[i]->height() > _height) _height = nscVec[i]->height();
    }
    //    _width = _width - nscVec.back()->width() - _hGap;
    _width = _width - _neighborGap + _hGap;
}

void NSContainer::vsreallocate()
{
    if (nscVec.size() == 0) {
      _width = _height = 0;
      return;
    }

    _height = _vGap;
    _width = 0;
    for (unsigned int i = 0; i < nscVec.size(); i++) {
      nscVec[i]->move(x() + _hGap, y() + _height);
      _height = _height + nscVec[i]->height() + _neighborGap;
      if (nscVec[i]->width() > _width) _width = nscVec[i]->width();
    }
    //    _height = _height - nscVec.back()->width() - _vGap;
    _height = _height - _neighborGap + _vGap;
}


// ##### NSStaticHContainer #####
void NSStaticHContainer::add(NSComponent* nsc)
{
  nsc->y(y() + vGap());
  if (size() == 0) {
    _width = hGap();
    _height = nsc->height() + vGap() * 2;
  } else {
    _width = _width - hGap() + neighborGap();
    if (_height < nsc->height()) _height = nsc->height() + vGap() * 2;
  }
  nsc->x(x() + _width);
  _width += nsc->width() + hGap();
  NSContainer::add(nsc);
}

void NSStaticHContainer::remove(NSComponent* nsc)
{
  NSContainer::hremove(nsc);
}

// ##### NSStaticVContainer #####
void NSStaticVContainer::add(NSComponent* nsc)
{
  nsc->x(x());
  if (size() == 0) {
    _height = vGap();
    _width = nsc->width() + hGap() * 2;
  } else {
    _height = _height - vGap() + neighborGap();
    if (_width < nsc->width()) _width = nsc->width() + hGap() * 2;
  }
  nsc->y(y() + _height);
  _height += nsc->height() + vGap();
  NSContainer::add(nsc);
}

void NSStaticVContainer::remove(NSComponent* nsc)
{
  NSContainer::vremove(nsc);
}
