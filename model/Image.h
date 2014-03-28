#pragma once

#include <boost/shared_ptr.hpp>
#include <memory>
#include <vector>

class Image
{
public:
  Image(boost::shared_ptr<unsigned char> pixelData,
  	    const int pixelLength);

  ~Image();

  void setPosition(const std::vector<double> pos);

  void setSize(const int width, const int height);

  // int pixelLength() const;

  int height() const;

  int width() const;
  
  boost::shared_ptr<unsigned char> pixelData() const;

private:
  class Pimpl;
  std::unique_ptr<Pimpl> _pimpl;
};
