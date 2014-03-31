#include "ImageStack.h"
#include "Image.h"

#include "util/DicomUtil.h"

#include <vector>
#include <algorithm>

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

bool compareImagePosition(boost::shared_ptr<const Image> image1, boost::shared_ptr<const Image> image2) {
  const Image::Orientation ori = image1->orientation();
  int index = 0;
  switch (ori) {
    case Image::TRAN:
    {
      index = 2;
      break;
    }
    case Image::CORO:
    {
      index = 1;
      break;
    }
    case Image::SAGI:
    {
     index = 0;
     break;
    }
    default:
    {
      std::cout<<"No image orientation information!"<<std::endl;
      return false;
    }
  }

  return (image1->position().at(index) > image2->position().at(index));
}

}

class ImageStack::Pimpl
{
public:
  Pimpl():currentIndex(0) {}

  /* data */
  std::vector<boost::shared_ptr<Image> > images;

  std::string imageFolder;

  unsigned int currentIndex;
};

ImageStack::ImageStack()
:_pimpl(new Pimpl()) {}

ImageStack::~ImageStack() {}

void ImageStack::loadImages(const std::string& imageFolder) {
  _pimpl->imageFolder = imageFolder;  
  _pimpl->images.clear();
  const std::vector<std::string> imageFiles = findImageFiles(imageFolder);
  for (auto imageFile : imageFiles) {
    DicomUtil dicomUtil(imageFile);
    if (dicomUtil.hasPixelData()) {
      _pimpl->images.push_back(dicomUtil.fetchImage());
    }
  }

  std::sort(_pimpl->images.begin(), _pimpl->images.end(), compareImagePosition);
}

boost::shared_ptr<const Image> ImageStack::fetchImage(int index) const {
  if (static_cast<unsigned int>(index) >= _pimpl->images.size() || index < 0) {
    return boost::shared_ptr<const Image>();
  }

  _pimpl->currentIndex = index;
  return _pimpl->images.at(index);
}

boost::shared_ptr<const Image> ImageStack::fetchNextImage() const {
  if (_pimpl->images.empty()) {
    return boost::shared_ptr<const Image>();
  }

  if (_pimpl->currentIndex < _pimpl->images.size() -1) {
    ++_pimpl->currentIndex; 
  }

  return _pimpl->images.at(_pimpl->currentIndex);
}

boost::shared_ptr<const Image> ImageStack::fetchPrevImage() const {
  if (_pimpl->images.empty()) {
    return boost::shared_ptr<const Image>();
  }

  if (_pimpl->currentIndex - 1 > 0) {
    --_pimpl->currentIndex; 
  }

  return _pimpl->images.at(_pimpl->currentIndex);
}

int ImageStack::imageCount() const {
  return _pimpl->images.size();
}
