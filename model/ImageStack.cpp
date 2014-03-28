#include "ImageStack.h"
#include "Image.h"

#include "util/DicomUtil.h"

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

  boost::filesystem::recursive_directory_iterator beg_iter(sourceFolder);
  const boost::filesystem::recursive_directory_iterator end_iter;
  for (;beg_iter != end_iter; ++beg_iter) {
    if (boost::filesystem::is_directory(*beg_iter)) {
      continue;
    }

    const std::string fileName = beg_iter->path().string();
    imageFiles.push_back(fileName);
  }

  return imageFiles;
}

}

class ImageStack::Pimpl
{
public:
  Pimpl() {}

  void loadImages() {
    const std::vector<std::string> imageFiles = findImageFiles(imageFolder);
    for (auto imageFile : imageFiles) {
      std::cout<<imageFile<<std::endl;
      DicomUtil dicomUtil(imageFile);
      if (dicomUtil.hasPixelData()) {
        images.push_back(dicomUtil.fetchImage());
        std::cout<<imageFile<<std::endl;
      }
    }
  }

  /* data */
  std::vector<boost::shared_ptr<Image> > images;

  std::string imageFolder;
};

ImageStack::ImageStack()
:_pimpl(new Pimpl()) {}

ImageStack::~ImageStack() {}

void ImageStack::setImageFolder(const std::string& imageFolder) {
  _pimpl->imageFolder = imageFolder;

  _pimpl->loadImages();
}

boost::shared_ptr<const Image> ImageStack::fetchImage(int index) const {
  if (index > _pimpl->images.size() || index < 0) {
    return boost::shared_ptr<const Image>();
  }

  return _pimpl->images.at(index);
}

int ImageStack::imageCount() const {
  return _pimpl->images.size();
}
