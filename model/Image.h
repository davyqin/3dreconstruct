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

  Image(boost::shared_ptr<unsigned char> pixelData,
  	    const int pixelLength);

  ~Image();

  void setPosition(const std::vector<double>& pos);

  void setOrientation(const std::vector<double>& ori);

  void setSize(const int width, const int height);

  int height() const;

  int width() const;
  
  boost::shared_ptr<unsigned char> pixelData() const;

  Orientation orientation() const;

  std::vector<double> position() const;

private:
  class Pimpl;
  std::unique_ptr<Pimpl> _pimpl;
};
