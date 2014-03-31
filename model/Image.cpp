#include "Image.h"

using namespace std;

namespace {
bool doublesAreEqual(double d1, double d2) {
  return std::fabs(d1 - d2) < std::numeric_limits<double>::epsilon();
}

const std::vector<double> transOri = {1.0, 0.0, 0.0, 0.0, 1.0, 0.0};
const std::vector<double> coroOri = {1.0, 0.0, 0.0, 0.0, 0.0, 1.0};
const std::vector<double> sagiOri = {0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
}

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
  Image::Orientation orientation;
  int width;
  int height;
};

Image::Image(boost::shared_ptr<unsigned char> pixelData, 
	           const int pixelLength)
:_pimpl(new Pimpl(pixelData, pixelLength)) {}

Image::~Image() {}

void Image::setPosition(const std::vector<double>& pos) {
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

void Image::setOrientation(const std::vector<double>& ori) {
  bool tran = true;
  bool coro = true;
  bool sagi = true;
  for (unsigned int i = 0; i < 6; ++i) {
    if (tran && !doublesAreEqual(ori.at(i), transOri.at(i))) {
      tran = false;
    }

    if (coro && !doublesAreEqual(ori.at(i), coroOri.at(i))) {
      coro = false;
    }

    if (sagi && !doublesAreEqual(ori.at(i), sagiOri.at(i))) {
      sagi = false;
    }
  }

  if (tran) _pimpl->orientation = TRAN;
  if (coro) _pimpl->orientation = CORO;
  if (sagi) _pimpl->orientation = SAGI;
}

Image::Orientation Image::orientation() const {
  return _pimpl->orientation;
}

std::vector<double> Image::position() const {
  return _pimpl->position;
}
