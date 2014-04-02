#pragma once

#include <boost/shared_ptr.hpp>
#include <memory>
#include <vector>

class Vertex;

class Cube
{
public:

  Cube();

  ~Cube();

private:
  class Pimpl;
  std::unique_ptr<Pimpl> _pimpl;
};
