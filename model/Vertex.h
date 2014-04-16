#pragma once

#include <boost/shared_ptr.hpp>
#include <memory>
#include <vector>

/*This class describes a vetex of an image. It contains the values and positions of 1 pixel on image. */
class Vertex
{
public:
  Vertex();

  Vertex(float x, float y, float z, int value = 0);

  Vertex(const Vertex& vertex);

  Vertex& operator=(const Vertex& vertex);

  ~Vertex();

  float x() const;

  float y() const;

  float z() const;

  std::vector<float> position() const;

  int value() const;

private:
  class Pimpl;
  std::unique_ptr<Pimpl> _pimpl;
};
