#include "Controller.h"

#include "Window.cpp"

#include <QDesktopWidget>
#include <QApplication>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

namespace {
 /**
 * Returns the file names of real dicom files in the @a sourceFolder.
 */
std::vector<std::string> findImageFiles(const std::string& path) {
  std::vector<std::string> imageFiles;

  const boost::filesystem::path sourceFolder(path);
  if (!boost::filesystem::exists(sourceFolder)) {
    return imageFiles;
  }

//  boost::filesystem::recursive_directory_iterator beg_iter(sourceFolder);
//  const boost::filesystem::recursive_directory_iterator end_iter;
//  for (;beg_iter != end_iter; ++beg_iter) {
//    if (boost::filesystem::is_directory(*beg_iter)) {
//      continue;
//    }

//    const std::string fileName = beg_iter->path().string();
//    // if (isDicomFile(fileName)) {
//    imageFiles.push_back(fileName);
//    // }
//  }
  return imageFiles;
}

}

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

