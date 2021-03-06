#include "McWorkshop.h"

#include "common.h"
#include "CubeFactory.h"
#include "model/Vertex.h"
#include "Triangle.h"
#include "Cube.h"

#include "model/ImageStack.h"
#include "model/Image.h"
#include "cuda/marching_cubes_kernel.h"

#include <boost/progress.hpp>
#include <boost/assign/list_of.hpp>
#include <vector>
#include <iostream>
#include <future>
#include <chrono>
#include <ctime>

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
                    const int minValue, const int maxValue) {
    std::vector<boost::shared_ptr<const Triangle> > triangles;
    for (unsigned int j = 0; j < cubes.size(); ++j) {
      Cube& cube = *cubes.at(j);
      int cubeindex = 0;
      if (cube.vertex(0)->value() >= minValue && cube.vertex(0)->value() <= maxValue) cubeindex |= 1;
      if (cube.vertex(1)->value() >= minValue && cube.vertex(1)->value() <= maxValue) cubeindex |= 2;
      if (cube.vertex(2)->value() >= minValue && cube.vertex(2)->value() <= maxValue) cubeindex |= 4;
      if (cube.vertex(3)->value() >= minValue && cube.vertex(3)->value() <= maxValue) cubeindex |= 8;
      if (cube.vertex(4)->value() >= minValue && cube.vertex(4)->value() <= maxValue) cubeindex |= 16;
      if (cube.vertex(5)->value() >= minValue && cube.vertex(5)->value() <= maxValue) cubeindex |= 32;
      if (cube.vertex(6)->value() >= minValue && cube.vertex(6)->value() <= maxValue) cubeindex |= 64;
      if (cube.vertex(7)->value() >= minValue && cube.vertex(7)->value() <= maxValue) cubeindex |= 128;

      if (EdgeTable[cubeindex] == 0 ) 
      {
        continue;
      }

      Vertex vertList[12];
      if ((EdgeTable[cubeindex] & 1) != 0) {
        vertList[0] = interpolate(*cube.vertex(0), *cube.vertex(1), minValue, maxValue);
      }

      if ((EdgeTable[cubeindex] & 2) != 0) {
        vertList[1] = interpolate(*cube.vertex(1), *cube.vertex(2), minValue, maxValue);
      }

      if ((EdgeTable[cubeindex] & 4) != 0) {
        vertList[2] = interpolate(*cube.vertex(2), *cube.vertex(3), minValue, maxValue);
      }

      if ((EdgeTable[cubeindex] & 8) != 0) {
        vertList[3] = interpolate(*cube.vertex(3), *cube.vertex(0), minValue, maxValue);
      }

      if ((EdgeTable[cubeindex] & 16) != 0) {
        vertList[4] = interpolate(*cube.vertex(4), *cube.vertex(5), minValue, maxValue);
      }

      if ((EdgeTable[cubeindex] & 32) != 0) {
        vertList[5] = interpolate(*cube.vertex(5), *cube.vertex(6), minValue, maxValue);
      }

      if ((EdgeTable[cubeindex] & 64) != 0) {
        vertList[6] = interpolate(*cube.vertex(6), *cube.vertex(7), minValue, maxValue);
      }

      if ((EdgeTable[cubeindex] & 128) != 0) {
        vertList[7] = interpolate(*cube.vertex(7), *cube.vertex(4), minValue, maxValue);
      }

      if ((EdgeTable[cubeindex] & 256) != 0) {
        vertList[8] = interpolate(*cube.vertex(0), *cube.vertex(4), minValue, maxValue);
      }

      if ((EdgeTable[cubeindex] & 512) != 0) {
        vertList[9] = interpolate(*cube.vertex(1), *cube.vertex(5), minValue, maxValue);
      }

      if ((EdgeTable[cubeindex] & 1024) != 0) {
        vertList[10] = interpolate(*cube.vertex(2), *cube.vertex(6), minValue, maxValue);
      }

      if ((EdgeTable[cubeindex] & 2048) != 0) {
        vertList[11] = interpolate(*cube.vertex(3), *cube.vertex(7), minValue, maxValue);
      }

      const int* vertices = TriangleTable[cubeindex];
      for (unsigned int i = 0; i < 16; ++i) {
        if (*(vertices + i) == -1) continue;
        const int a = *(vertices + i);
        const int b = *(vertices + i + 1);
        const int c = *(vertices + i + 2);

        std::vector<boost::shared_ptr<const Vertex> > temp;
        temp.push_back(boost::shared_ptr<const Vertex>(new Vertex(vertList[a])));
        temp.push_back(boost::shared_ptr<const Vertex>(new Vertex(vertList[b])));
        temp.push_back(boost::shared_ptr<const Vertex>(new Vertex(vertList[c])));
        triangles.push_back(boost::shared_ptr<const Triangle>(new Triangle(temp)));
        i = i + 2;
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

std::vector<boost::shared_ptr<const Triangle> > McWorkshop::work(int engine) {
  if (!_pimpl->triangles.empty()) return _pimpl->triangles;
  _pimpl->imageStack->setOrientation(0);  // force to use axial images
  const int imageCount = _pimpl->imageStack->imageCount();
  if (imageCount == 0) {
    return std::vector<boost::shared_ptr<const Triangle> >();
  }

  cout<<endl<<"Generating triangles..."<<endl;
  cout << "Min: " << _pimpl->minValue << " Max: " << _pimpl->maxValue;
  std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();

  if (engine == 1) {
    cout << " Quality: 1" << " Enginee: CUDA/GPU" << endl;
    // Initialize CUDA buffers for Marching Cubes
    boost::shared_ptr<const Image> tmpImage = _pimpl->imageStack->fetchImage(0);
    initMC(_pimpl->minValue, _pimpl->maxValue, tmpImage->width(), 2, tmpImage->pixelSpacing().at(0), tmpImage->pixelSpacing().at(1), tmpImage->pixelSpacing().at(2));
    boost::progress_display pd(imageCount - 1);
    const int imageSize = tmpImage->width() * tmpImage->height();
    const int bufferSize = imageSize * 2;
    boost::shared_ptr<unsigned char> buffer(new unsigned char[bufferSize]);
    const int maxVerts = 512 * 512 * 50;
    boost::shared_ptr<float> xPos(new float[maxVerts]);
    boost::shared_ptr<float> yPos(new float[maxVerts]);
    boost::shared_ptr<float> zPos(new float[maxVerts]);
    unsigned int totalVertices = 0;
    for (int i = 0; i < (imageCount - 1); ++i) {
      boost::shared_ptr<const Image> bottomImage = _pimpl->imageStack->fetchImage(i);
      boost::shared_ptr<const Image> topImage = _pimpl->imageStack->fetchImage(i+1);

      memcpy(&(buffer.get()[0]), bottomImage->pixelData8bit().get(), imageSize);
      memcpy(&(buffer.get()[imageSize]), topImage->pixelData8bit().get(), imageSize); 

      const std::vector<double> startPos = bottomImage->position();
      totalVertices = 0;
      computeIsosurface(buffer.get(), startPos.at(0), startPos.at(1), startPos.at(2), xPos.get(), yPos.get(), zPos.get(), totalVertices);

      const unsigned int startIndex = 0;
      const unsigned int endIndex = totalVertices;
      for (unsigned int j = startIndex; j < endIndex; ++j) {
        std::vector<boost::shared_ptr<const Vertex> > vertices;
        vertices.push_back(boost::shared_ptr<Vertex>(new Vertex(xPos.get()[j], yPos.get()[j], zPos.get()[j])));
        ++j;
        vertices.push_back(boost::shared_ptr<Vertex>(new Vertex(xPos.get()[j], yPos.get()[j], zPos.get()[j])));
        ++j;
        vertices.push_back(boost::shared_ptr<Vertex>(new Vertex(xPos.get()[j], yPos.get()[j], zPos.get()[j])));
        _pimpl->triangles.push_back(boost::shared_ptr<Triangle>(new Triangle(vertices)));
      }
      ++pd;
    }
    cleanup();
  }

  if (engine == 0) {
    cout << " Quality: "<< _pimpl->quality << " Enginee: CPU" << endl;
    const unsigned int maxWorkerSize = std::thread::hardware_concurrency() * 2;
    std::vector<std::future<std::vector<boost::shared_ptr<const Triangle> > > > workerPool;
  
    boost::progress_display pd(imageCount);
    for (int i = 0; i < (imageCount - 1); ++i) {
      boost::shared_ptr<const Image> bottomImage = _pimpl->imageStack->fetchImage(i);
      boost::shared_ptr<const Image> topImage = _pimpl->imageStack->fetchImage(i+1);
      const CubeFactory cubeFactory(bottomImage, topImage);

      std::future<std::vector<boost::shared_ptr<const Triangle> > > worker = 
          std::async(std::launch::async, [=](){return generateTriangles(cubeFactory.cubes(),
                                                                        _pimpl->minValue,
                                                                        _pimpl->maxValue);});
      workerPool.push_back(std::move(worker));
    
      if (workerPool.size() == maxWorkerSize) {
        for (auto& worker: workerPool) {
          std::vector<boost::shared_ptr<const Triangle> > temp = worker.get();
          _pimpl->triangles.insert(_pimpl->triangles.end(), temp.begin(), temp.end());
          ++pd;
        }
        workerPool.clear();
      }
    }

    if (!workerPool.empty()) {
      for (auto& worker: workerPool) {
        const std::vector<boost::shared_ptr<const Triangle> > temp = worker.get();
        _pimpl->triangles.insert(_pimpl->triangles.end(), temp.begin(), temp.end());
        ++pd;
      }
      workerPool.clear();
    }
  }

  std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end - start;
  std::cout<<std::endl<<"Finish generating triangels in "<<elapsed_seconds.count()<<" s"<<std::endl;

  return _pimpl->triangles;
}
