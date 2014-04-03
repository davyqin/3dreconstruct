#include "Image.h"

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
  std::vector<double> pixelSpacing;

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

std::vector<double> Image::position() const {
  return _pimpl->position;
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

int Image::pixelLength() const {
  return _pimpl->pixelLength;
}

boost::shared_ptr<unsigned short> Image::pixelData() const {
  if (!_pimpl->outputPixel) {
    int nCount = _pimpl->pixelLength;
    _pimpl->outputPixel.reset(new unsigned short[_pimpl->pixelLength + 16]);
    unsigned short* np = _pimpl->outputPixel.get();
    unsigned short* pp = _pimpl->pixelData.get();

    const double dSlope = 65535.0 / (double)_pimpl->window;
    const double dShift = (double)_pimpl->level - (double)_pimpl->window / 2.0;

    while (nCount-- > 0) {
      int value = ((int)*pp - dShift) * dSlope;
      if (value <= 0) {
        value = 0;
      }
      else if (value >= 65535) {
        value = 65535;
      }
      
      *np = static_cast<unsigned short>(value);
      ++np; ++pp;      
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

unsigned short Image::maxValue() const {
  return _pimpl->maxValue;
}

unsigned short Image::minValue() const {
  return _pimpl->minValue;
}

unsigned short Image::pixelValue(unsigned int index) const {
  return *(_pimpl->pixelData.get() + index);
}

void Image::updateWL(int window, int level) {
  bool wlChanged = false;
  if (_pimpl->window != window) {
    _pimpl->window = window;
    wlChanged = true;
  }

  if (_pimpl->level != level) {
    _pimpl->level = level;
    wlChanged = true;
  }

  if (wlChanged) _pimpl->outputPixel.reset();
}

void Image::setPixelSpacing(const std::vector<double>& value) {
  _pimpl->pixelSpacing = value;
}

std::vector<double> Image::pixelSpacing() const {
  return _pimpl->pixelSpacing;
}
