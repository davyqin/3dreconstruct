#ifndef IMAGESTACK_H
#define IMAGESTACK_H

#include <boost/shared_ptr.hpp>
#include <memory>
#include <string>
#include <vector>

class Image;

class ImageStack
{
public:
  ImageStack();

  ~ImageStack();

  void loadImages(const std::string& imageFolder);

  boost::shared_ptr<const Image> fetchImage(int index) const;

  boost::shared_ptr<const Image> fetchImage() const;

  int imageCount() const;

  void updateWL(int window, int level);

  void set3dQuality(int value);

  void setOrientation(int index);

  std::vector<boost::shared_ptr<const Image> > images() const;

private:
  class Pimpl;
  std::unique_ptr<Pimpl> _pimpl;
};

#endif // IMAGESTACK_H
