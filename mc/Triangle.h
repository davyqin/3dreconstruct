#pragma once

#include "Vertex.h"

#include <boost/shared_ptr.hpp>
#include <memory>
#include <vector>

class Vertex;

class Triangle
{
public:
  Triangle(const std::vector<Vertex> vertices);

  Triangle(const Triangle& triangle);

  Triangle& operator=(const Triangle& triangle);

  ~Triangle();

  std::vector<Vertex> vertices() const;

  std::vector<double> normals() const;

private:
  class Pimpl;
  std::unique_ptr<Pimpl> _pimpl;
};
