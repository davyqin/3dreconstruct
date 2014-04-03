#include "Controller.h"

#include "ViewDialog.cpp"
#include "model/ImageStack.h"

/**********************************/
#include "mc/CubeFactory.h"
#include "mc/Cube.h"
/**********************************/

#include <QDesktopWidget>
#include <QApplication>

#include <boost/shared_ptr.hpp>

using namespace std;

class Controller::Pimpl {
public:
  Pimpl() 
  :imageStack(new ImageStack()) {}
  ViewDialog viewDialog;
  boost::shared_ptr<ImageStack> imageStack;
};

Controller::Controller(QObject *parent) 
	: QObject(parent), _pimpl(new Pimpl())
{
  connect(&_pimpl->viewDialog, SIGNAL(loadImageSignal(const QString&)), SLOT(onLoadImage(const QString&)));
  connect(&_pimpl->viewDialog, SIGNAL(requestImage(int)), SLOT(onRequestImage(int)));
  connect(&_pimpl->viewDialog, SIGNAL(updateWLSignal(int,int)), SLOT(onUpdateWL(int,int)));
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
  _pimpl->imageStack->loadImages(imageFolder.toStdString());
  if (_pimpl->imageStack->imageCount() > 0) {
    _pimpl->viewDialog.setImageCount(_pimpl->imageStack->imageCount());
    onRequestImage(0);
  }
}

void Controller::onRequestImage(int index) {
  _pimpl->viewDialog.showImage(_pimpl->imageStack->fetchImage(index));
}

namespace {
  // void testCubeFactory(const ImageStack& imageStack) {
  //   CubeFactory cubeFactory;
  //   cubeFactory.setImages(imageStack.fetchImage(0), imageStack.fetchImage(1));
  //   const std::vector<Cube> cubes = cubeFactory.cubes();
  //   const Cube cube = cubes.front();
  //   const std::vector<Vertex> vertices = cube.vertices();
  //   for (auto vertex : vertices) {
  //     cout<<vertex.x()<<" "<<vertex.y()<<" "<<vertex.z()<<std::endl;
  //   }
  // }
}

void Controller::onUpdateWL(int window, int level) {
  _pimpl->imageStack->updateWL(window, level);
  _pimpl->viewDialog.showImage(_pimpl->imageStack->fetchImage());
  //testCubeFactory(_pimpl->imageStack);
}
