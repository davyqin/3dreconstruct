#pragma once

#include <boost/shared_ptr.hpp>
#include <memory>
#include <vector>

class Cube;

class CubeFactory
{
public:

  CubeFactory();

  ~CubeFactory();

private:
  class Pimpl;
  std::unique_ptr<Pimpl> _pimpl;
};
