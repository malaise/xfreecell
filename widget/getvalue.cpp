#include <cstdio>
#include <cstdlib>

#include "widget.h"

class SeedWindow : public NSFrame, public NSButtonListener {
public:
  SeedWindow();

  void buttonAction(const XEvent&, void*);
  int value() const { return atoi(textField.text()); }
  bool ok() const { return _okPressed; }
  void waitForEvent();

private:
  NSHContainer buttonCon;
  NSVContainer mainCon;
  NSLabel label;
  NSButton okButton, cancelButton;
  NSTextField textField;

  bool _okPressed, _cancelPressed;
};

SeedWindow::SeedWindow()
  : buttonCon(200, 50),
    mainCon(200, 120),
    label("Type game number"),
    okButton(" Ok ", this),
    cancelButton("Cancel", this),
    textField(150)
{
  buttonCon.add(&okButton);
  buttonCon.add(&cancelButton);
  buttonCon.reallocate();

  mainCon.add(&label);
  mainCon.add(&textField);
  mainCon.add(&buttonCon);
  mainCon.reallocate();

  container(&mainCon);

  _okPressed = false;
  _cancelPressed = false;
}

void SeedWindow::buttonAction(const XEvent& ev, void*)
{
  if (ev.xany.window == okButton.window())
    _okPressed = true;
  else if (ev.xany.window == cancelButton.window())
    _cancelPressed = true;
}

void SeedWindow::waitForEvent()
{
  _okPressed = false;
  _cancelPressed = false;

  map();
  raise();

  XEvent ev;

  while (!_okPressed && !_cancelPressed) {
    NSNextEvent( &ev);
    NSDispatchEvent(ev);
  }

  unmap();
}

int main()
{
  NSInitialize();

  SeedWindow sw;

  sw.waitForEvent();

  printf("%d\n", sw.value());

  return 0;
}
