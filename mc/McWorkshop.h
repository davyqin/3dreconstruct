#pragma once

#include "Triangle.h"

#include <boost/shared_ptr.hpp>
#include <memory>


class ImageStack;

class McWorkshop
{
public:
  McWorkshop();

  McWorkshop(boost::shared_ptr<const ImageStack> imageStack);

  ~McWorkshop();

  void setMinValue(int value);

  void setMaxValue(int value);

  void setIsoMinMax(int minValue, int maxValue);

  void setImageStack(boost::shared_ptr<const ImageStack> imageStack);

  std::vector<boost::shared_ptr<const Triangle> > work();

private:
  class Pimpl;
  std::unique_ptr<Pimpl> _pimpl;
};