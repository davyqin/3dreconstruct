#include "Controller.h"

#include "ViewDialog.h"
#include "View3DDialog.h"
#include "model/ImageStack.h"
#include "mc/McWorkshop.h"

#include <QDesktopWidget>
#include <QApplication>

#include <boost/shared_ptr.hpp>
#include <boost/progress.hpp>

using namespace std;

class Controller::Pimpl {
public:
  Pimpl() 
  :imageStack(new ImageStack()) {}
  ViewDialog viewDialog;
  View3DDialog view3dDialog;
  boost::shared_ptr<ImageStack> imageStack;
};

Controller::Controller(QObject *parent) 
	: QObject(parent), _pimpl(new Pimpl())
{
  connect(&_pimpl->viewDialog, SIGNAL(loadImageSignal(const QString&)), SLOT(onLoadImage(const QString&)));
  connect(&_pimpl->viewDialog, SIGNAL(requestImage(int)), SLOT(onRequestImage(int)));
  connect(&_pimpl->viewDialog, SIGNAL(updateWLSignal(int,int)), SLOT(onUpdateWL(int,int)));
  connect(&_pimpl->viewDialog, SIGNAL(show3dSignal()), SLOT(onShow3d()));
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

 _pimpl->viewDialog.update();
}

void Controller::onLoadImage(const QString& imageFolder) {
  boost::progress_timer timer;
  _pimpl->imageStack->loadImages(imageFolder.toStdString());
  if (_pimpl->imageStack->imageCount() > 0) {
    _pimpl->viewDialog.setImageCount(_pimpl->imageStack->imageCount());
    onRequestImage(0);
  }
}

void Controller::onRequestImage(int index) {
  _pimpl->viewDialog.showImage(_pimpl->imageStack->fetchImage(index));
}

void Controller::onUpdateWL(int window, int level) {
  _pimpl->imageStack->updateWL(window, level);
  _pimpl->viewDialog.showImage(_pimpl->imageStack->fetchImage());
}

void Controller::onShow3d() {
  boost::progress_timer timer;
  const McWorkshop mcWorkshop(_pimpl->imageStack);
  const std::vector<boost::shared_ptr<const Triangle> > data = mcWorkshop.work();
  if (!data.empty()) {
    _pimpl->view3dDialog.show();
    _pimpl->view3dDialog.show3D(data);
  }
}