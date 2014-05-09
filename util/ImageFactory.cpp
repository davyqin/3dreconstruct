#include "ImageFactory.h"

#include "model/Image.h"

#include <limits>
#include <iostream>

using namespace std;

namespace {
  template<typename T>
  boost::shared_ptr<T> resizeBilinearGray(boost::shared_ptr<T> image, int w, int h, int w2, int h2) {
    boost::shared_ptr<T> temp(new T[w2*h2]);
    int A, B, C, D, x, y, index, gray ;
    float x_ratio = ((float)(w-1))/w2 ;
    float y_ratio = ((float)(h-1))/h2 ;
    float x_diff, y_diff;//, ya, yb ;
    int offset = 0 ;
    T* pixels = (T*)image.get();
    T* newPixel = (T*)temp.get();
    for (int i=0;i<h2;i++) {
      for (int j=0;j<w2;j++) {
        x = (int)(x_ratio * j) ;
        y = (int)(y_ratio * i) ;
        x_diff = (x_ratio * j) - x ;
        y_diff = (y_ratio * i) - y ;
        index = y*w+x ;

        // range is 0 to 255 thus bitwise AND with 0xff
        A = *(pixels + index) & std::numeric_limits<T>::max();
        B = *(pixels + index + 1) & std::numeric_limits<T>::max();
        C = *(pixels + index + w) & std::numeric_limits<T>::max();
        D = *(pixels + index + w + 1) & std::numeric_limits<T>::max();
            
        // Y = A(1-w)(1-h) + B(w)(1-h) + C(h)(1-w) + Dwh
        gray = (T)(
          A*(1-x_diff)*(1-y_diff) +  B*(x_diff)*(1-y_diff) +
          C*(y_diff)*(1-x_diff)   +  D*(x_diff*y_diff)
          ) ;

        *(newPixel + offset) = gray;
        ++offset;
      }
    }
  return temp;
}

}

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
