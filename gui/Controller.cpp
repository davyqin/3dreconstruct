#include "Controller.h"

#include "ViewDialog.h"
#include "View3DDialog.h"
#include "VolumeRenderingDialog.h"
#include "model/ImageStack.h"
#include "mc/McWorkshop.h"

#include <QDesktopWidget>
#include <QApplication>

#include <boost/shared_ptr.hpp>
#include <boost/progress.hpp>

#include <chrono>
#include <ctime>

using namespace std;

class Controller::Pimpl {
public:
  Pimpl() 
  :imageStack(new ImageStack()) {}
  ViewDialog viewDialog;
  View3DDialog view3dDialog;
  VolumeRenderingDialog volumeRenderingDialog;
  boost::shared_ptr<ImageStack> imageStack;
};

Controller::Controller(QObject *parent) 
	: QObject(parent), _pimpl(new Pimpl())
{
  connect(&_pimpl->viewDialog, SIGNAL(loadImageSignal(const QString&)), SLOT(onLoadImage(const QString&)));
  connect(&_pimpl->viewDialog, SIGNAL(requestImage(int)), SLOT(onRequestImage(int)));
  connect(&_pimpl->viewDialog, SIGNAL(updateWLSignal(int,int)), SLOT(onUpdateWL(int,int)));
  connect(&_pimpl->viewDialog, SIGNAL(orientationSignal(int)), SLOT(onOrientation(int)));

  connect(&_pimpl->view3dDialog, SIGNAL(show3DSignal(int,int,int,int)), SLOT(onShow3d(int,int,int,int)));
  connect(&_pimpl->volumeRenderingDialog, SIGNAL(show3DSignal()), SLOT(onVolumeRendering()));
}

Controller::~Controller() {}

void Controller::activate() {
  _pimpl->volumeRenderingDialog.adjustSize();
  _pimpl->volumeRenderingDialog.show();
  _pimpl->volumeRenderingDialog.resize(650,650);

  _pimpl->view3dDialog.adjustSize();
  _pimpl->view3dDialog.show();
  _pimpl->view3dDialog.resize(650,650);

  _pimpl->viewDialog.adjustSize();
  _pimpl->viewDialog.show();
  _pimpl->viewDialog.resize(650,650);
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

void Controller::onShow3d(int minValue, int maxValue, int qualityValue, int engine) {
  std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();

  McWorkshop mcWorkshop(_pimpl->imageStack);
  mcWorkshop.set3dQuality(qualityValue);
  mcWorkshop.setIsoMinMax(minValue, maxValue);
  _pimpl->view3dDialog.show3D(mcWorkshop.work(engine));

  std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end - start;
  std::cout<<std::endl<<"Finish Controller::onShow3d(...) in "<<elapsed_seconds.count()<<" s"<<std::endl;
}

void Controller::onOrientation(int index) {
  _pimpl->imageStack->setOrientation(index);
  _pimpl->viewDialog.setImageCount(_pimpl->imageStack->imageCount());
  _pimpl->viewDialog.showImage(_pimpl->imageStack->fetchImage());
}

void Controller::onVolumeRendering() {
  std::vector<boost::shared_ptr<const Image> > images = std::move(_pimpl->imageStack->images());
  _pimpl->volumeRenderingDialog.show3D(images);
}