#include "Controller.h"

#include "Window.cpp"

#include <QDesktopWidget>
#include <QApplication>

class Controller::Pimpl {
public:
  Window dialog;
};

Controller::Controller(QObject *parent) 
	: QObject(parent), _pimpl(new Pimpl())
{
}

Controller::~Controller() {}

void Controller::activate() {
  const int desktopArea = QApplication::desktop()->width() *  QApplication::desktop()->height();
  const int widgetArea = _pimpl->dialog.width() * _pimpl->dialog.height();
  if (((float)widgetArea / (float)desktopArea) < 0.75f)
    _pimpl->dialog.show();
  else
    _pimpl->dialog.showMaximized();
}

