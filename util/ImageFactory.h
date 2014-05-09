#pragma once

#include <boost/shared_ptr.hpp>
#include <memory>
#include <vector>

class Image;

class ImageFactory
{
public:
  ImageFactory(std::vector<boost::shared_ptr<const Image> >& images);

  ~ImageFactory();

  std::vector<boost::shared_ptr<Image> > coronalImages();

  std::vector<boost::shared_ptr<Image> > sagittalImages();

private:
  class Pimpl;
  std::unique_ptr<Pimpl> _pimpl;
};
