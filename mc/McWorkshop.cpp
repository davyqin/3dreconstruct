#include "McWorkshop.h"

#include "common.h"
#include "Grid.h"
#include "McFactory.h"
#include "CubeFactory.h"
#include "model/Vertex.h"
#include "Triangle.h"
#include "Cube.h"

#include "model/ImageStack.h"

#include <boost/progress.hpp>
#include <vector>
#include <iostream>
#include <functional>
#include <future>

using namespace std;

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

  std::vector<boost::shared_ptr<const Triangle> > 
  generateTriangles(const std::vector<boost::shared_ptr<Cube> >& cubes,
                    const int minValue, const int maxValue,
                    const unsigned int startPos, const unsigned int endPos/*,
                    boost::progress_display& pd*/) {
    std::vector<boost::shared_ptr<const Triangle> > triangles;
    for (unsigned int i = startPos; i <= endPos; ++i) {
      Cube& cube = *cubes.at(i);
      int cubeindex = 0;
      if (cube.vertex(0).value() >= minValue && cube.vertex(0).value() <= maxValue) cubeindex |= 1;
      if (cube.vertex(1).value() >= minValue && cube.vertex(1).value() <= maxValue) cubeindex |= 2;
      if (cube.vertex(2).value() >= minValue && cube.vertex(2).value() <= maxValue) cubeindex |= 4;
      if (cube.vertex(3).value() >= minValue && cube.vertex(3).value() <= maxValue) cubeindex |= 8;
      if (cube.vertex(4).value() >= minValue && cube.vertex(4).value() <= maxValue) cubeindex |= 16;
      if (cube.vertex(5).value() >= minValue && cube.vertex(5).value() <= maxValue) cubeindex |= 32;
      if (cube.vertex(6).value() >= minValue && cube.vertex(6).value() <= maxValue) cubeindex |= 64;
      if (cube.vertex(7).value() >= minValue && cube.vertex(7).value() <= maxValue) cubeindex |= 128;

      if (EdgeTable[cubeindex] == 0 ) 
      {
        continue;
      }

      Vertex vertList[12];
      if ((EdgeTable[cubeindex] & 1) != 0) {
        vertList[0] = interpolate(cube.vertex(0), cube.vertex(1), minValue, maxValue);
      }

      if ((EdgeTable[cubeindex] & 2) != 0) {
        vertList[1] = interpolate(cube.vertex(1), cube.vertex(2), minValue, maxValue);
      }

      if ((EdgeTable[cubeindex] & 4) != 0) {
        vertList[2] = interpolate(cube.vertex(2), cube.vertex(3), minValue, maxValue);
      }

      if ((EdgeTable[cubeindex] & 8) != 0) {
        vertList[3] = interpolate(cube.vertex(3), cube.vertex(0), minValue, maxValue);
      }

      if ((EdgeTable[cubeindex] & 16) != 0) {
        vertList[4] = interpolate(cube.vertex(4), cube.vertex(5), minValue, maxValue);
      }

      if ((EdgeTable[cubeindex] & 32) != 0) {
        vertList[5] = interpolate(cube.vertex(5), cube.vertex(6), minValue, maxValue);
      }

      if ((EdgeTable[cubeindex] & 64) != 0) {
        vertList[6] = interpolate(cube.vertex(6), cube.vertex(7), minValue, maxValue);
      }

      if ((EdgeTable[cubeindex] & 128) != 0) {
        vertList[7] = interpolate(cube.vertex(7), cube.vertex(4), minValue, maxValue);
      }

      if ((EdgeTable[cubeindex] & 256) != 0) {
        vertList[8] = interpolate(cube.vertex(0), cube.vertex(4), minValue, maxValue);
      }

      if ((EdgeTable[cubeindex] & 512) != 0) {
        vertList[9] = interpolate(cube.vertex(1), cube.vertex(5), minValue, maxValue);
      }

      if ((EdgeTable[cubeindex] & 1024) != 0) {
        vertList[10] = interpolate(cube.vertex(2), cube.vertex(6), minValue, maxValue);
      }

      if ((EdgeTable[cubeindex] & 2048) != 0) {
        vertList[11] = interpolate(cube.vertex(3), cube.vertex(7), minValue, maxValue);
      }

      const vector<int> vertices = TriangleTable[cubeindex];
      for (unsigned int i = 0; i < vertices.size(); ++i) {
        if (vertices.at(i) == -1) continue;

        triangles.push_back(
          boost::shared_ptr<const Triangle>(new Triangle({vertList[vertices.at(i)], 
                                                          vertList[vertices.at(++i)],
                                                          vertList[vertices.at(++i)]})));
      }
    }

    return triangles;
  }
} // namespace

class McWorkshop::Pimpl
{
public:
  Pimpl() 
  :minValue(0), maxValue(0) {}

  Pimpl(boost::shared_ptr<ImageStack> imageStack)
  :imageStack(imageStack),minValue(0),maxValue(0),quality(4) {}

  /* data */
  boost::shared_ptr<ImageStack> imageStack;
  int minValue;
  int maxValue;
  int quality;
  std::vector<boost::shared_ptr<const Triangle> > triangles;

  std::vector<boost::shared_ptr<const Triangle> > 
  triangleTask(const std::vector<boost::shared_ptr<Cube> >& cubes) const {
   std::vector<boost::shared_ptr<const Triangle> > triangles;
   unsigned int groupSize = std::thread::hardware_concurrency();
   if (groupSize < 4) groupSize = 4;
   const unsigned int cubeSize = cubes.size();
   const unsigned int numberOfCubes = std::ceil(cubeSize / groupSize);
   auto triangleFunc = std::bind(generateTriangles, std::ref(cubes), minValue, maxValue,
                                 std::placeholders::_1, std::placeholders::_2);

   unsigned int startPos = 0;
   unsigned int endPos = numberOfCubes;
   std::vector<std::future<std::vector<boost::shared_ptr<const Triangle> > > > workerPool;
   while (endPos < cubeSize) {
     std::future<std::vector<boost::shared_ptr<const Triangle> > > worker = 
       std::async(std::launch::async, [=](){return triangleFunc(startPos, endPos);});

      workerPool.push_back(std::move(worker));
      startPos = endPos + 1;
      endPos = startPos + numberOfCubes;
    }

    std::future<std::vector<boost::shared_ptr<const Triangle> > > worker = 
      std::async(std::launch::async, [=](){return triangleFunc(startPos, (cubeSize - 1));});
    workerPool.push_back(std::move(worker));

    for (auto& worker: workerPool) {
      const std::vector<boost::shared_ptr<const Triangle> > temp = worker.get();
      triangles.insert(triangles.end(), temp.begin(), temp.end());
    }

    return triangles;
  }
};

McWorkshop::McWorkshop()
:_pimpl(new Pimpl()) {}

McWorkshop::McWorkshop(boost::shared_ptr<ImageStack> imageStack) 
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

void McWorkshop::set3dQuality(int value) {
  if (_pimpl->quality == value) return;

  _pimpl->quality = value;
  _pimpl->imageStack->set3dQuality(value);
  _pimpl->triangles.clear();
}

void McWorkshop::setImageStack(boost::shared_ptr<ImageStack> imageStack) {
  _pimpl->imageStack = imageStack;
  _pimpl->triangles.clear();
}

std::vector<boost::shared_ptr<const Triangle> > McWorkshop::work() {
  if (!_pimpl->triangles.empty()) return _pimpl->triangles;
  _pimpl->imageStack->setOrientation(0);  // force to use axial images
  const int imageCount = _pimpl->imageStack->imageCount();
  if (imageCount == 0) {
    return std::vector<boost::shared_ptr<const Triangle> >();
  }

  cout<<endl<<"Calculating triangles..."<<endl;
  cout<<"Min: "<<_pimpl->minValue<<" Max: "<<_pimpl->maxValue<<" Quality: "<<_pimpl->quality<<endl;

  boost::progress_timer timer;
  boost::progress_display pd(imageCount);
  for (int i = 0; i < (imageCount - 1); ++i) {
    boost::shared_ptr<const Image> bottomImage = _pimpl->imageStack->fetchImage(i);
    boost::shared_ptr<const Image> topImage = _pimpl->imageStack->fetchImage(i+1);
    CubeFactory cubeFactory(bottomImage, topImage);
    const std::vector<boost::shared_ptr<const Triangle> >& temp = _pimpl->triangleTask(cubeFactory.cubes());
    _pimpl->triangles.insert(_pimpl->triangles.end(), temp.begin(), temp.end());
    ++pd;
  }

  return _pimpl->triangles;
}
