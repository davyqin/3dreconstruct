#pragma once

#include <boost/shared_ptr.hpp>
#include <memory>

class Grid;
class ImageStack;

class McWorkshop
{
public:

  McWorkshop(boost::shared_ptr<const ImageStack> imageStack);

  ~McWorkshop();

  void work() const;

private:
  class Pimpl;
  std::unique_ptr<Pimpl> _pimpl;
};