#include "ImageFactory.h"

#include "model/Image.h"

#include <iostream>

using namespace std;

class ImageFactory::Pimpl {
public:
  Pimpl(std::vector<boost::shared_ptr<const Image> >& images)
  :axialImages(images) {}

  std::vector<boost::shared_ptr<const Image> > axialImages;
  std::vector<boost::shared_ptr<Image> > coronalImages;
  std::vector<boost::shared_ptr<Image> > sagittalImages;

  void generateCoronal() {
  	if (axialImages.empty()) return;
  	const int coronalHeight = axialImages.size();
  	const int coronalWidth = axialImages.at(0)->width();
  	const int pixelLength = coronalWidth * coronalHeight;
  	const int count = axialImages.at(0)->height();

  	std::vector<boost::shared_ptr<unsigned short> > newShortPixelDatas;
  	std::vector<boost::shared_ptr<unsigned char> > newCharPixelDatas;
  	for (int i = 0; i < count; ++i) {
  	  newShortPixelDatas.push_back(boost::shared_ptr<unsigned short>(new unsigned short[pixelLength + 16]));
  	  newCharPixelDatas.push_back(boost::shared_ptr<unsigned char>(new unsigned char[pixelLength + 8]));
  	}

  	const int axialWidth = axialImages.at(0)->width();
  	const int axialHeight = axialImages.at(0)->height();
  	int index = 0;
  	for (auto image : axialImages) {
  	  const unsigned short* axialShortPixel = image->rawPixelData().get();
  	  const unsigned char* axialCharPixel = image->rawPixelData8bit().get();
  	  for (int i = 0; i < axialHeight; ++i) {
  	  	unsigned short* newShortPixelData = newShortPixelDatas.at(i).get();
  	  	unsigned char* newCharPixelData = newCharPixelDatas.at(i).get();
  	    for (int j = 0; j < axialWidth; ++j) {
  	      const int coronalIndex = index * coronalWidth + j;
          const int axialIndex = i * axialWidth + j;
  	  	  *(newShortPixelData + coronalIndex) = *(axialShortPixel + axialIndex);
  	  	  *(newCharPixelData + coronalIndex) = *(axialCharPixel + axialIndex);
  	    }
  	  }
  	  ++index;
  	}

  	for (int i = 0; i < count; ++i) {
  	  boost::shared_ptr<Image> image(new Image(newShortPixelDatas.at(i), newCharPixelDatas.at(i), pixelLength));
  	  image->setSize(coronalWidth, coronalHeight);
  	  coronalImages.push_back(image);
  	}
  }

  void generateSagittal() {
    if (axialImages.empty()) return;
    const int sagittalHeight = axialImages.size();
    const int sagittalWidth = axialImages.at(0)->height();
    const int pixelLength = sagittalWidth * sagittalHeight;
    const int count = axialImages.at(0)->height();

    std::vector<boost::shared_ptr<unsigned short> > newShortPixelDatas;
    std::vector<boost::shared_ptr<unsigned char> > newCharPixelDatas;
    for (int i = 0; i < count; ++i) {
      newShortPixelDatas.push_back(boost::shared_ptr<unsigned short>(new unsigned short[pixelLength + 16]));
      newCharPixelDatas.push_back(boost::shared_ptr<unsigned char>(new unsigned char[pixelLength + 8]));
    }

    const int axialWidth = axialImages.at(0)->width();
    const int axialHeight = axialImages.at(0)->height();
    int index = 0;
    for (auto image : axialImages) {
      const unsigned short* axialShortPixel = image->rawPixelData().get();
      const unsigned char* axialCharPixel = image->rawPixelData8bit().get();
      for (int i = 0; i < axialHeight; ++i) {
        unsigned short* newShortPixelData = newShortPixelDatas.at(i).get();
        unsigned char* newCharPixelData = newCharPixelDatas.at(i).get();
        for (int j = 0; j < axialWidth; ++j) {
          const int sagittalIndex = index * sagittalWidth + j;
          const int axialIndex = (axialHeight - j) * axialWidth + i;
          *(newShortPixelData + sagittalIndex) = *(axialShortPixel + axialIndex);
          *(newCharPixelData + sagittalIndex) = *(axialCharPixel + axialIndex);
        }
      }
      ++index;
    }

    for (int i = 0; i < count; ++i) {
      boost::shared_ptr<Image> image(new Image(newShortPixelDatas.at(i), newCharPixelDatas.at(i), pixelLength));
      image->setSize(sagittalWidth, sagittalHeight);
      sagittalImages.push_back(image);
    }
  }
};

ImageFactory::ImageFactory(std::vector<boost::shared_ptr<const Image> >& images) 
: _pimpl(new Pimpl(images)) {}

ImageFactory::~ImageFactory() {}

std::vector<boost::shared_ptr<Image> > 
ImageFactory::coronalImages() {
  if (_pimpl->coronalImages.empty()) {
  	_pimpl->generateCoronal();
  }

  return _pimpl->coronalImages;
}

std::vector<boost::shared_ptr<Image> > 
ImageFactory::sagittalImages() {
  if (_pimpl->sagittalImages.empty()) {
    _pimpl->generateSagittal();
  }

  return _pimpl->sagittalImages;
}
