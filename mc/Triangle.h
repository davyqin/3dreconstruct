#pragma once

#include "model/Vertex.h"

#include <boost/shared_ptr.hpp>
#include <memory>
#include <vector>

#define GLM_FORCE_CXX11
#include <glm/glm.hpp>

class Vertex;

class Triangle
{
public:
  Triangle(const std::vector<Vertex> vertices);

  Triangle(const Triangle& triangle);

  Triangle& operator=(const Triangle& triangle);

  ~Triangle();

  std::vector<Vertex> vertices() const;

  std::vector<float> normal() const;

private:
  class Pimpl;
  std::unique_ptr<Pimpl> _pimpl;
};
