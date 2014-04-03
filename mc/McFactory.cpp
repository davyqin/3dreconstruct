#include "McFactory.h"

#include "model/ImageStack.h"

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
