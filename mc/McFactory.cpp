#include "McFactory.h"

#include "Grid.h"
#include "Cube.h"
#include "CubeFactory.h"
#include "model/ImageStack.h"
#include "model/Image.h"

#include <boost/progress.hpp>
#include <vector>
#include <iostream>

using namespace std;

class McFactory::Pimpl
{
public:
  Pimpl(boost::shared_ptr<const ImageStack> imageStack)
  :imageStack(imageStack) {}

  /* data */
  boost::shared_ptr<const ImageStack> imageStack;
};

McFactory::McFactory(boost::shared_ptr<const ImageStack> imageStack)
:_pimpl(new Pimpl(imageStack)) {}

McFactory::~McFactory() {}

boost::shared_ptr<Grid> McFactory::grid() const {
  const int imageCount = _pimpl->imageStack->imageCount();
  if (imageCount == 0) {
    return boost::shared_ptr<Grid>();
  }

    boost::shared_ptr<Grid> grid(new Grid());

  cout<<endl<<"Generating cubes..."<<endl;
  boost::progress_display pd(imageCount);
  std::vector<boost::shared_ptr<const Cube> > cubes;
  for (int i = 0; i < imageCount; ++i) {
    int index = i;
    boost::shared_ptr<const Image> bottomImage = _pimpl->imageStack->fetchImage(index);
    boost::shared_ptr<const Image> topImage = _pimpl->imageStack->fetchImage(++index);
    CubeFactory cubeFactory;
    cubeFactory.setImages(bottomImage, topImage);
    const std::vector<boost::shared_ptr<const Cube> > temp = cubeFactory.cubes();
    cubes.insert(cubes.end(), temp.begin(), temp.end());
    ++pd;
  }

  grid->setCubes(cubes);

  // assume the orientation of image is transverse.
  grid->set3D(_pimpl->imageStack->fetchImage(0)->width()/2,
             _pimpl->imageStack->fetchImage(0)->height()/2,
             imageCount - 1);

  return grid;
}
