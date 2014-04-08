#pragma once

#include "Triangle.h"

#include <boost/shared_ptr.hpp>
#include <memory>


class ImageStack;

class McWorkshop
{
public:

  McWorkshop(boost::shared_ptr<const ImageStack> imageStack);

  ~McWorkshop();

  std::vector<Triangle> work() const;

private:
  class Pimpl;
  std::unique_ptr<Pimpl> _pimpl;
};