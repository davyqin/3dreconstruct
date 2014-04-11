#include "McWorkshop.h"

#include "common.h"
#include "Grid.h"
#include "McFactory.h"
#include "model/Vertex.h"
#include "Triangle.h"
#include "Cube.h"

#include "model/ImageStack.h"

#include <boost/progress.hpp>
#include <vector>
#include <iostream>

using namespace std;

class McWorkshop::Pimpl
{
public:
  Pimpl() 
  :minValue(0), maxValue(0) {}

  Pimpl(boost::shared_ptr<const ImageStack> imageStack)
  :imageStack(imageStack),minValue(0),maxValue(0) {}

  /* data */
  boost::shared_ptr<const ImageStack> imageStack;
  int minValue;
  int maxValue;
  boost::shared_ptr<const Grid> grid;
  std::vector<boost::shared_ptr<const Triangle> > triangles;
};

McWorkshop::McWorkshop()
:_pimpl(new Pimpl()) {}

McWorkshop::McWorkshop(boost::shared_ptr<const ImageStack> imageStack) 
:_pimpl(new Pimpl(imageStack)) {}

McWorkshop::~McWorkshop() {}

void McWorkshop::setIsoMinMax(int minValue, int maxValue) {
  if (minValue == _pimpl->minValue && 
      maxValue == _pimpl->maxValue) {
    return;
  }

  _pimpl->minValue = minValue;
  _pimpl->maxValue = maxValue;
  _pimpl->triangles.clear();
}

void McWorkshop::setImageStack(boost::shared_ptr<const ImageStack> imageStack) {
  _pimpl->imageStack = imageStack;
  _pimpl->grid.reset();
  _pimpl->triangles.clear();
}

namespace {
  Vertex interpolate(const Vertex& vertex1, const Vertex& vertex2, double minValue, double maxValue) {
    const double dv = vertex1.value() - vertex2.value();
    const double dValMax1 = vertex1.value() - maxValue;
    const double dMaxVal2 = maxValue - vertex2.value();
    const double dValMin1 = vertex1.value() - minValue;
    const double dMinVal2 = minValue - vertex2.value();

    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    if (vertex1.value() >= minValue && vertex1.value() <= maxValue) {
      if (vertex2.value() > maxValue) {
        x = (vertex2.x() * dValMax1 + vertex1.x() * dMaxVal2) / dv;
        y = (vertex2.y() * dValMax1 + vertex1.y() * dMaxVal2) / dv;
        z = (vertex2.z() * dValMax1 + vertex1.z() * dMaxVal2) / dv;
      }
      else {
        x = (vertex2.x() * dValMin1 + vertex1.x() * dMinVal2) / dv;
        y = (vertex2.y() * dValMin1 + vertex1.y() * dMinVal2) / dv;
        z = (vertex2.z() * dValMin1 + vertex1.z() * dMinVal2) / dv;
      }
    }
    else if (vertex1.value() > maxValue) {
      x = (vertex2.x() * dValMax1 + vertex1.x() * dMaxVal2) / dv;
      y = (vertex2.y() * dValMax1 + vertex1.y() * dMaxVal2) / dv;
      z = (vertex2.z() * dValMax1 + vertex1.z() * dMaxVal2) / dv;
    }
    else {
      x = (vertex2.x() * dValMin1 + vertex1.x() * dMinVal2) / dv;
      y = (vertex2.y() * dValMin1 + vertex1.y() * dMinVal2) / dv;
      z = (vertex2.z() * dValMin1 + vertex1.z() * dMinVal2) / dv;
    }

    return Vertex(x, y, z);
  }
}

std::vector<boost::shared_ptr<const Triangle> > McWorkshop::work() {
  if (!_pimpl->triangles.empty()) return _pimpl->triangles;

  if (!_pimpl->grid) {
    const McFactory mcFactory(_pimpl->imageStack);
    _pimpl->grid = boost::shared_ptr<const Grid>(new Grid(mcFactory.grid()));
  }

  const std::vector<boost::shared_ptr<const Cube> > cubes = _pimpl->grid->cubes();
  cout<<endl<<"Calculating triangles..."<<endl;
  cout<<"Min: "<<_pimpl->minValue<<" Max: "<<_pimpl->maxValue<<endl;
  boost::progress_display pd(cubes.size());
  for (unsigned int i = 0; i < cubes.size(); ++i) {
    const Cube& cube = *cubes.at(i);
    int cubeindex = 0;
    if (cube.vertex(0).value() >= _pimpl->minValue && cube.vertex(0).value() <= _pimpl->maxValue) cubeindex |= 1;
    if (cube.vertex(1).value() >= _pimpl->minValue && cube.vertex(1).value() <= _pimpl->maxValue) cubeindex |= 2;
    if (cube.vertex(2).value() >= _pimpl->minValue && cube.vertex(2).value() <= _pimpl->maxValue) cubeindex |= 4;
    if (cube.vertex(3).value() >= _pimpl->minValue && cube.vertex(3).value() <= _pimpl->maxValue) cubeindex |= 8;
    if (cube.vertex(4).value() >= _pimpl->minValue && cube.vertex(4).value() <= _pimpl->maxValue) cubeindex |= 16;
    if (cube.vertex(5).value() >= _pimpl->minValue && cube.vertex(5).value() <= _pimpl->maxValue) cubeindex |= 32;
    if (cube.vertex(6).value() >= _pimpl->minValue && cube.vertex(6).value() <= _pimpl->maxValue) cubeindex |= 64;
    if (cube.vertex(7).value() >= _pimpl->minValue && cube.vertex(7).value() <= _pimpl->maxValue) cubeindex |= 128;

    ++pd;

    if (EdgeTable[cubeindex] == 0 ) continue;

    Vertex vertList[12];
    if ((EdgeTable[cubeindex] & 1) != 0) {
      vertList[0] = interpolate(cube.vertex(0), cube.vertex(1), _pimpl->minValue, _pimpl->maxValue);
    }

    if ((EdgeTable[cubeindex] & 2) != 0) {
      vertList[1] = interpolate(cube.vertex(1), cube.vertex(2), _pimpl->minValue, _pimpl->maxValue);
    }

    if ((EdgeTable[cubeindex] & 4) != 0) {
      vertList[2] = interpolate(cube.vertex(2), cube.vertex(3), _pimpl->minValue, _pimpl->maxValue);
    }

    if ((EdgeTable[cubeindex] & 8) != 0) {
      vertList[3] = interpolate(cube.vertex(3), cube.vertex(0), _pimpl->minValue, _pimpl->maxValue);
    }

    if ((EdgeTable[cubeindex] & 16) != 0) {
      vertList[4] = interpolate(cube.vertex(4), cube.vertex(5), _pimpl->minValue, _pimpl->maxValue);
    }

    if ((EdgeTable[cubeindex] & 32) != 0) {
      vertList[5] = interpolate(cube.vertex(5), cube.vertex(6), _pimpl->minValue, _pimpl->maxValue);
    }

    if ((EdgeTable[cubeindex] & 64) != 0) {
      vertList[6] = interpolate(cube.vertex(6), cube.vertex(7), _pimpl->minValue, _pimpl->maxValue);
    }

    if ((EdgeTable[cubeindex] & 128) != 0) {
      vertList[7] = interpolate(cube.vertex(7), cube.vertex(4), _pimpl->minValue, _pimpl->maxValue);
    }

    if ((EdgeTable[cubeindex] & 256) != 0) {
      vertList[8] = interpolate(cube.vertex(0), cube.vertex(4), _pimpl->minValue, _pimpl->maxValue);
    }

    if ((EdgeTable[cubeindex] & 512) != 0) {
      vertList[9] = interpolate(cube.vertex(1), cube.vertex(5), _pimpl->minValue, _pimpl->maxValue);
    }

    if ((EdgeTable[cubeindex] & 1024) != 0) {
      vertList[10] = interpolate(cube.vertex(2), cube.vertex(6), _pimpl->minValue, _pimpl->maxValue);
    }

    if ((EdgeTable[cubeindex] & 2048) != 0) {
      vertList[11] = interpolate(cube.vertex(3), cube.vertex(7), _pimpl->minValue, _pimpl->maxValue);
    }

    const vector<int> vertices = TriangleTable[cubeindex];
    for (unsigned int i = 0; i < vertices.size(); ++i) {
      if (vertices.at(i) == -1) continue;

      _pimpl->triangles.push_back(
        boost::shared_ptr<const Triangle>(new Triangle({vertList[vertices.at(i)], 
                                                        vertList[vertices.at(++i)],
                                                        vertList[vertices.at(++i)]})));
    }
  }

  return _pimpl->triangles;
}
