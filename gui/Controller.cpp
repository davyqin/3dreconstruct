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
  McWorkshop mcWorkshop;
};

Controller::Controller(QObject *parent) 
	: QObject(parent), _pimpl(new Pimpl())
{
  connect(&_pimpl->viewDialog, SIGNAL(loadImageSignal(const QString&)), SLOT(onLoadImage(const QString&)));
  connect(&_pimpl->viewDialog, SIGNAL(requestImage(int)), SLOT(onRequestImage(int)));
  connect(&_pimpl->viewDialog, SIGNAL(updateWLSignal(int,int)), SLOT(onUpdateWL(int,int)));

  connect(&_pimpl->view3dDialog, SIGNAL(show3DSignal(int,int,int)), SLOT(onShow3d(int,int,int)));
}

Controller::~Controller() {}

void Controller::activate() {
  _pimpl->view3dDialog.show();
  _pimpl->view3dDialog.adjustSize();

  _pimpl->viewDialog.show();
  _pimpl->viewDialog.adjustSize();
}

void Controller::onLoadImage(const QString& imageFolder) {
  boost::progress_timer timer;
  _pimpl->imageStack->loadImages(imageFolder.toStdString());
  if (_pimpl->imageStack->imageCount() > 0) {
    _pimpl->mcWorkshop.setImageStack(_pimpl->imageStack);
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

void Controller::onShow3d(int minValue, int maxValue, int qualityValue) {
  boost::progress_timer timer;
  _pimpl->mcWorkshop.set3dQuality(qualityValue);
  _pimpl->mcWorkshop.setIsoMinMax(minValue, maxValue);
  _pimpl->view3dDialog.show3D(_pimpl->mcWorkshop.work());
}
