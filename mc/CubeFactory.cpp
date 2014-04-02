#include "CubeFactory.h"
#include "Cube.h"


using namespace std;

class CubeFactory::Pimpl
{
public:
  Pimpl() {}

  /* data */
  vector<Cube> cubes;
};

CubeFactory::CubeFactory()
:_pimpl(new Pimpl()) {}

CubeFactory::~CubeFactory() {}
