#pragma once

#include <boost/shared_ptr.hpp>
#include <memory>
#include <vector>

/*This class describes a vetex of an image. It contains the values and positions of 1 pixel on image. */
class Vertex
{
public:
  Vertex();

  Vertex(double x, double y, double z, int value = 0);

  Vertex(const Vertex& vertex);

  Vertex& operator=(const Vertex& vertex);

  ~Vertex();

  double x() const;

  double y() const;

  double z() const;

  void setPosition(double x, double y, double z);

  std::vector<double> position() const;

  int value() const;

private:
  class Pimpl;
  std::unique_ptr<Pimpl> _pimpl;
};
