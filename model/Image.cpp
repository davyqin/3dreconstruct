#include "Image.h"

#include "cuda/cuda_info.h"
#include "cuda/cuda_kernels.h"

#include <vector>
#include <boost/assign/list_of.hpp>

using namespace std;

namespace {
bool doublesAreEqual(double d1, double d2) {
  return std::fabs(d1 - d2) < std::numeric_limits<double>::epsilon();
}

const std::vector<double> transOri = boost::assign::list_of(1.0)(0.0)(0.0)(0.0)(1.0)(0.0);
const std::vector<double> coroOri = boost::assign::list_of(1.0)(0.0)(0.0)(0.0)(0.0)(1.0);
const std::vector<double> sagiOri = boost::assign::list_of(0.0)(1.0)(0.0)(0.0)(0.0)(1.0);
}

class Image::Pimpl
{
public:
  Pimpl(boost::shared_ptr<unsigned short> pixelData,
        boost::shared_ptr<unsigned char> pixelData8bit,
  	    const int pixelLength)
  : pixelData(pixelData)
  , pixelData8bit(pixelData8bit)
  , pixelLength(pixelLength)
  , window(65535)
  , level(32768)
  , sampleStep(4)
  , dataType(Image::SHORTBIT)
  , xPos(boost::shared_ptr<float>())
  , yPos(boost::shared_ptr<float>())
  {
//    computerMinAndMax();
  }

  /* data */
  boost::shared_ptr<unsigned short> pixelData;
  boost::shared_ptr<unsigned char> pixelData8bit;
  const unsigned long pixelLength;
  std::vector<double> position;
  Image::Orientation orientation;
  int width;
  int height;
  unsigned short minValue;
  unsigned short maxValue;
  int window;
  int level;
  int sampleStep;
  std::vector<double> pixelSpacing;
  boost::shared_ptr<unsigned short> outputPixel;
  boost::shared_ptr<unsigned char> outputPixel8bit;
  Image::DataType dataType;
  boost::shared_ptr<float> xPos;
  boost::shared_ptr<float> yPos;

  std::vector<boost::shared_ptr<const Vertex> > generateVertices() {
    const double xInc = pixelSpacing.at(0);
    const double yInc = pixelSpacing.at(1);
    const int cols = width;
    const int rows = height;
    std::vector<boost::shared_ptr<const Vertex> > vertices;

#if 1
    const int size = width * height;
    float zPos = position.at(2);

    for (int i = 0; i < rows; i += sampleStep) {
      for (int j = 0; j < cols; j += sampleStep) {
        const unsigned int index = i * rows + j;
        const int value = *(pixelData8bit.get() + index);
        vertices.push_back(boost::shared_ptr<Vertex>(new Vertex(xPos.get()[index], yPos.get()[index], zPos, value)));
      }
    }

#endif


#if 0
    for (int i = 0; i < rows; i += sampleStep) {
      for (int j = 0; j < cols; j += sampleStep) {
        const unsigned int index = i * rows + j;
        const int value = *(pixelData8bit.get() + index);
        vector<double> vertexPosition = position;
        switch (orientation)
        {
          case (Image::TRAN):
          {
            vertexPosition.at(0) += (j * xInc);
            vertexPosition.at(1) += (i * yInc);
            break;
          }
          default:
          {
            // Add codes later
            break;
          }
        }
        vertices.push_back(
          boost::shared_ptr<Vertex>(new Vertex(vertexPosition.at(0), vertexPosition.at(1), 
                                               vertexPosition.at(2), value)));
      }
    }
#endif
    return vertices;
  }

#if 0
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
#endif
};

Image::Image(boost::shared_ptr<unsigned short> pixelData,
             boost::shared_ptr<unsigned char> pixelData8bit,
	           const int pixelLength)
:_pimpl(new Pimpl(pixelData, pixelData8bit, pixelLength)) {}

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

int Image::height() const {
  return _pimpl->height;
}

int Image::width() const {
  return _pimpl->width;
}

int Image::pixelLength() const {
  return _pimpl->pixelLength;
}

boost::shared_ptr<const unsigned short> Image::rawPixelData() const {
  return _pimpl->pixelData;
}

boost::shared_ptr<const unsigned char> Image::rawPixelData8bit() const {
  return _pimpl->pixelData8bit;
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

boost::shared_ptr<unsigned char> Image::pixelData8bit() const {
  if (!_pimpl->outputPixel8bit) {
    int nCount = _pimpl->pixelLength;
    _pimpl->outputPixel8bit.reset(new unsigned char[_pimpl->pixelLength + 16]);
    unsigned char* np = _pimpl->outputPixel8bit.get();
    unsigned char* pp = _pimpl->pixelData8bit.get();

    // const double dSlope = 65535.0 / (double)_pimpl->window;
    // const double dShift = (double)_pimpl->level - (double)_pimpl->window / 2.0;

    while (nCount-- > 0) {
      // int value = ((int)*pp - dShift) * dSlope;
      // if (value <= 0) {
      //   value = 0;
      // }
      // else if (value >= 65535) {
      //   value = 65535;
      // }
      
      *np++ = *pp++;
    }
  }

  return _pimpl->outputPixel8bit;
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

  if (wlChanged) {
    _pimpl->outputPixel.reset();
  }
}

void Image::setPixelSpacing(const std::vector<double>& value) {
  _pimpl->pixelSpacing = value;
}

std::vector<double> Image::pixelSpacing() const {
  return _pimpl->pixelSpacing;
}

std::vector<boost::shared_ptr<const Vertex> > Image::vertices() const {
  if (!_pimpl->pixelData && !_pimpl->outputPixel) {
    return std::vector<boost::shared_ptr<const Vertex> >();
  }

  return  _pimpl->generateVertices();
}

int Image::sampleStep() const {
  return _pimpl->sampleStep;
}

void Image::setSampleStep(const int step) {
  if (_pimpl->sampleStep != step) {
    _pimpl->sampleStep = step;
  }
}

Image::DataType Image::dataType() const {
  return _pimpl->dataType;
}

void Image::setDataType(const Image::DataType dataType) {
  _pimpl->dataType = dataType;
}

void Image::calcVetexPos() {
  if (_pimpl->xPos.get() != nullptr || _pimpl->yPos.get() != nullptr) return;

  const double xInc = _pimpl->pixelSpacing.at(0);
  const double yInc = _pimpl->pixelSpacing.at(1);
  const int cols = _pimpl->width;
  const int rows = _pimpl->height;

  const int size = _pimpl->width * _pimpl->height;
  _pimpl->xPos.reset(new float[size]);
  _pimpl->yPos.reset(new float[size]);

  // CUDA not efficient now
  // calcPos(_pimpl->width, _pimpl->height, _pimpl->position.at(0), _pimpl->position.at(1),
     //      xInc, yInc, size, _pimpl->xPos.get(), _pimpl->yPos.get());

  // CPU
  for (int i = 0; i < rows; i += 1) {
    for (int j = 0; j < cols; j += 1) {
      const unsigned int index = i * rows + j;
      _pimpl->xPos.get()[index] = _pimpl->position.at(0) + xInc * j;
      _pimpl->yPos.get()[index] = _pimpl->position.at(1) +  yInc * i;
    }
  }
}
