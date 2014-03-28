#include "Controller.h"

#include "ViewDialog.cpp"
#include "model/ImageStack.h"

#include <QDesktopWidget>
#include <QApplication>

class Controller::Pimpl {
public:
  ViewDialog viewDialog;
  ImageStack imageStack;
};

Controller::Controller(QObject *parent) 
	: QObject(parent), _pimpl(new Pimpl())
{
  connect(&_pimpl->viewDialog, SIGNAL(loadImageSignal(const QString&)), SLOT(onLoadImage(const QString&)));
}

Controller::~Controller() {}

void Controller::activate() {
  _pimpl->viewDialog.adjustSize();
  const int desktopArea = QApplication::desktop()->width() *  QApplication::desktop()->height();
  const int widgetArea = _pimpl->viewDialog.width() * _pimpl->viewDialog.height();
  if (((float)widgetArea / (float)desktopArea) < 0.75f)
    _pimpl->viewDialog.show();
  else
    _pimpl->viewDialog.showMaximized();
}

void Controller::onLoadImage(const QString& imageFolder) {
  _pimpl->imageStack.setImageFolder(imageFolder.toStdString());
}

