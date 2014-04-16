#pragma once

#include <boost/shared_ptr.hpp>
#include <memory>
#include <vector>

class Cube;
class Image;

class CubeFactory
{
public:

  CubeFactory();

  ~CubeFactory();

  void setImages(boost::shared_ptr<const Image> downsideImage, 
                 boost::shared_ptr<const Image> upsideImage);

  std::vector<boost::shared_ptr<Cube> > cubes() const;

private:
  class Pimpl;
  std::unique_ptr<Pimpl> _pimpl;
};
