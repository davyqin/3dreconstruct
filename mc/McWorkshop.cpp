#include "McWorkshop.h"

#include "common.h"
#include "Grid.h"
#include "McFactory.h"
#include "model/ImageStack.h"

#include <vector>
#include <iostream>

using namespace std;

class McWorkshop::Pimpl
{
public:
  Pimpl(boost::shared_ptr<const ImageStack> imageStack)
  :imageStack(imageStack) {}

  /* data */
  boost::shared_ptr<const ImageStack> imageStack;
};

McWorkshop::McWorkshop(boost::shared_ptr<const ImageStack> imageStack) 
:_pimpl(new Pimpl(imageStack)) {}

McWorkshop::~McWorkshop() {}

void McWorkshop::work() const {
  if (!_pimpl->imageStack || _pimpl->imageStack->imageCount() == 0) {
  	cout<<"No images loaded, quit workshop"<<endl;
  	return;
  }

  const McFactory mcFactory(_pimpl->imageStack);
  const Grid grid = mcFactory.grid();

  const double isoValue = 40000.0;
  const std::vector<Cube> cubes = grid.cubes();
  for (auto cube : cubes) {
    int cubeindex = 0;
    for (int i = 7; i >= 0; --i) {
      cout << cube.vertex(i).value()<<endl;
      // if (cube.vertex(i).value() < isoValue) {
      //   cubeindex &= 1<<i;
      // }
      // else {
      //   cubeindex &= 0<<i;
      // }
    }
    // cout<<cubeindex<<endl;
  }
}
