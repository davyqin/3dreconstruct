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
  Pimpl(boost::shared_ptr<unsigned short> pixelData,
  	    const int pixelLength)
  : pixelData(pixelData)
  , pixelLength(pixelLength)
  , window(65535)
  , level(32768)
  {
//    computerMinAndMax();
  }

  /* data */
  boost::shared_ptr<unsigned short> pixelData;
  boost::shared_ptr<unsigned short> outputPixel;
  const unsigned long pixelLength;
  std::vector<double> position;
  Image::Orientation orientation;
  int width;
  int height;
  unsigned short minValue;
  unsigned short maxValue;
  int window;
  int level;

  void computerMinAndMax() {
    minValue = std::numeric_limits<unsigned short>::max();
    maxValue = std::numeric_limits<unsigned short>::min();
    unsigned short* p = pixelData.get();

    for (unsigned long i = 0; i < pixelLength; ++i, ++p)
    {
      if (*p < minValue) minValue = *p;
      if (*p > maxValue) maxValue = *p;
    }
  }
};

Image::Image(boost::shared_ptr<unsigned short> pixelData, 
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

boost::shared_ptr<unsigned short> Image::pixelData() const {
  if (!_pimpl->outputPixel) {
    int nCount = _pimpl->pixelLength;
    _pimpl->outputPixel.reset(new unsigned short[_pimpl->pixelLength + 16]);
    unsigned short* np = _pimpl->outputPixel.get();
    unsigned short* pp = _pimpl->pixelData.get();

    const double dSlope = 65535.0f / _pimpl->window;
    const unsigned short halfWindow = _pimpl->window / 2.0;
    const unsigned short leftWindow = _pimpl->level >  halfWindow ? _pimpl->level - halfWindow : 0;
    const unsigned short rightWindow = (65535 - _pimpl->level) > halfWindow ? _pimpl->level + halfWindow : 65535;

    while (nCount-- > 0) {
      if (*pp <= leftWindow) {
        *np = 0;
      }
      else if (*pp >= rightWindow) {
        *np = 65535;
      }
      else {
        *np = (*pp - leftWindow) * dSlope;
      }
      pp++;
      np++;       
    }
  }

  return _pimpl->outputPixel;
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

unsigned short Image::maxValue() const {
  return _pimpl->maxValue;
}

unsigned short Image::minValue() const {
  return _pimpl->minValue;
}

void Image::updateWL(int window, int level) {
  _pimpl->window = window;
  _pimpl->level = level;
  _pimpl->outputPixel.reset();
}
