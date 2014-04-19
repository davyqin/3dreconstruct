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

  CubeFactory(boost::shared_ptr<const Image> downImage, 
              boost::shared_ptr<const Image> upImage);

  CubeFactory(const CubeFactory& other);

  ~CubeFactory();

  void setImages(boost::shared_ptr<const Image> downImage, 
                 boost::shared_ptr<const Image> upImage);

  boost::shared_ptr<const Image> downImage() const;

  boost::shared_ptr<const Image> upImage() const;

  std::vector<boost::shared_ptr<Cube> > cubes() const;

private:
  class Pimpl;
  std::unique_ptr<Pimpl> _pimpl;
};
