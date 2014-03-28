#include "ImageStack.h"
#include "Image.h"

#include <vector>

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

class ImageStack::Pimpl
{
public:
  Pimpl() {}

  void loadImages() {

  }

  /* data */
  std::vector<Image> images;

  std::string imageFolder;
};

ImageStack::ImageStack()
:_pimpl(new Pimpl()) {}

ImageStack::~ImageStack() {}

void ImageStack::setImageFolder(const std::string& imageFolder) {
  _pimpl->imageFolder = imageFolder;

  _pimpl->loadImages();
}

boost::shared_ptr<Image> ImageStack::fetchImage(int index) {
  return boost::shared_ptr<Image>();
}

