#include "Image.h"

using namespace std;

class Image::Pimpl
{
public:
  Pimpl(boost::shared_ptr<unsigned char> pixelData,
  	    const int pixelLength)
  : pixelData(pixelData)
  , pixelLength(pixelLength) {}

  /* data */
  boost::shared_ptr<unsigned char> pixelData;
  const int pixelLength;
  std::vector<double> position;
  int width;
  int height;
};

Image::Image(boost::shared_ptr<unsigned char> pixelData, 
	         const int pixelLength)
:_pimpl(new Pimpl(pixelData, pixelLength)) {}

Image::~Image() {}

void Image::setPosition(const std::vector<double> pos) {
  _pimpl->position = pos;
}

void Image::setSize(const int width, const int height) {
  _pimpl->width = width;
  _pimpl->height = height;
}

  // int pixelLength() const;

int Image::height() const {
  return _pimpl->height;
}

int Image::width() const {
  return _pimpl->width;
}

boost::shared_ptr<unsigned char> Image::pixelData() const {
  return _pimpl->pixelData;
}
