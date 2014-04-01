#pragma once

#include <boost/shared_ptr.hpp>
#include <memory>
#include <vector>

class Image
{
public:
  enum Orientation {
      TRAN,
      CORO,
      SAGI
  };

  Image(boost::shared_ptr<unsigned short> pixelData,
        const int pixelLength);

  ~Image();

  void setPosition(const std::vector<double>& pos);

  void setOrientation(const std::vector<double>& ori);

  void setSize(const int width, const int height);

  void setPixelSpacing(const std::vector<double>& value);

  int height() const;

  int width() const;
  
  boost::shared_ptr<unsigned short> pixelData() const;

  Orientation orientation() const;

  std::vector<double> position() const;

  unsigned short maxValue() const;

  unsigned short minValue() const;

  void updateWL(int window, int level);

private:
  class Pimpl;
  std::unique_ptr<Pimpl> _pimpl;
};
