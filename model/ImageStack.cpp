#include "ImageStack.h"
#include "Image.h"

#include "util/DicomUtil.h"
#include "util/ImageFactory.h"

#include <vector>
#include <algorithm>
#include <future>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/progress.hpp>

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

  return (image1->position().at(index) < image2->position().at(index));
}

}

class ImageStack::Pimpl
{
public:
  Pimpl()
  :currentIndex(0)
  ,minLevel(std::numeric_limits<int>::max())
  ,maxLevel(std::numeric_limits<int>::min())
  ,quality(4)
  ,ori(Image::TRAN) {}

  /* data */
  std::vector<boost::shared_ptr<Image> > images;
  std::vector<boost::shared_ptr<Image> > coronalImages;
  std::vector<boost::shared_ptr<Image> > sagittalImages;
  std::string imageFolder;
  unsigned int currentIndex;
  int minLevel;
  int maxLevel;
  int quality;
  Image::Orientation ori;

  std::vector<boost::shared_ptr<Image> > activeImages() {
    std::vector<boost::shared_ptr<Image> > activeimages;
    switch (ori) {
      case Image::SAGI:
      {
        activeimages = sagittalImages;
        break;
      }
      case Image::CORO:
      {
        activeimages = coronalImages;
        break;
      }
      default:
      {
        activeimages = images;
        break;
      }
    }
    return activeimages;
  }
};

ImageStack::ImageStack()
:_pimpl(new Pimpl()) {}

ImageStack::~ImageStack() {}

namespace {
std::vector<std::vector<std::string> > groupFileNames(
  const std::vector<std::string>& fileNames/*, unsigned int groupSize*/)
{
  const unsigned int groupSize = std::thread::hardware_concurrency();
  unsigned int numberOfFiles = std::ceil(fileNames.size() / groupSize);
  std::vector<std::vector<std::string> > fileGroups;
  
  std::vector<std::string> temp;
  for (auto file : fileNames) {
    temp.push_back(file);
    if (temp.size() == numberOfFiles) {
      fileGroups.push_back(temp);
      temp.clear();
    }
  }

  if (!temp.empty()) {
    fileGroups.push_back(temp);
  }

  return fileGroups;
}
}

void ImageStack::loadImages(const std::string& imageFolder) {
  if (_pimpl->imageFolder == imageFolder) return;
  
  _pimpl->imageFolder = imageFolder;  
  _pimpl->images.clear();
  _pimpl->currentIndex = 0;

  const std::vector<std::string> imageFiles = std::move(findImageFiles(imageFolder));
  boost::progress_display pd(imageFiles.size());

  const std::vector<std::vector<std::string> > fileGroups = std::move(groupFileNames(imageFiles));
  std::vector<std::future<std::vector<boost::shared_ptr<Image> > > > workerPool;
  for (auto fileGroup : fileGroups) {
    std::future<std::vector<boost::shared_ptr<Image> > > worker = 
    std::async(std::launch::async, [fileGroup, &pd]() {
      std::vector<boost::shared_ptr<Image> > images;
      for (auto file : fileGroup) {
        DicomUtil dicomUtil(file);
        if (dicomUtil.hasPixelData()) {
          images.push_back(dicomUtil.fetchImage());
        }
        ++pd;
      }  
      return images;
    });

    workerPool.push_back(std::move(worker));
  }

  for (auto& worker: workerPool) {
    const std::vector<boost::shared_ptr<Image> > temp = worker.get();
    _pimpl->images.insert(_pimpl->images.end(), temp.begin(), temp.end());
  }

  std::sort(_pimpl->images.begin(), _pimpl->images.end(), compareImagePosition);

  std::vector<boost::shared_ptr<const Image> > tempImages;
  tempImages.insert(tempImages.end(), _pimpl->images.begin(), _pimpl->images.end());

  ImageFactory imageFactory(tempImages);
  _pimpl->coronalImages = imageFactory.coronalImages();
  _pimpl->sagittalImages = imageFactory.sagittalImages();
}

boost::shared_ptr<const Image> ImageStack::fetchImage(int index) const {
  std::vector<boost::shared_ptr<Image> > activeImages = std::move(_pimpl->activeImages());

  if (static_cast<unsigned int>(index) >= activeImages.size() || index < 0) {
    return boost::shared_ptr<const Image>();
  }

  _pimpl->currentIndex = index;
  return activeImages.at(index);
}

boost::shared_ptr<const Image> ImageStack::fetchImage() const {
  std::vector<boost::shared_ptr<Image> > activeImages = std::move(_pimpl->activeImages());

  if (activeImages.empty()) {
    return boost::shared_ptr<const Image>();
  }

  return activeImages.at(_pimpl->currentIndex);
}

int ImageStack::imageCount() const {
  if (_pimpl->images.empty()) {
    return 0;
  }
  
  return _pimpl->images.size();
}

void ImageStack::updateWL(int window, int level) {
  for (auto image : _pimpl->images) {
    image->updateWL(window, level);
  }
}

void ImageStack::set3dQuality(int value) {
  for (auto image : _pimpl->images) {
    image->setSampleStep(value);
  }
}

void ImageStack::setOrientation(int index) {
  _pimpl->ori = static_cast<Image::Orientation>(index);
}