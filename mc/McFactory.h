#pragma once

#include <boost/shared_ptr.hpp>
#include <memory>

class Grid;
class ImageStack;

class McFactory
{
public:

  McFactory(boost::shared_ptr<const ImageStack> imageStack);

  ~McFactory();

  Grid grid() const;

private:
  class Pimpl;
  std::unique_ptr<Pimpl> _pimpl;
};
