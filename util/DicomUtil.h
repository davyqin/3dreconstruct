#ifndef DICOMUTIL_H
#define DICOMUTIL_H

#include <boost/shared_ptr.hpp>

class Image;

class DicomUtil
{
public:

    virtual ~DicomUtil() {}

    virtual boost::shared_ptr<unsigned short> pixel() = 0;

    virtual boost::shared_ptr<Image> fetchImage() const = 0;

    virtual int pixelLength() const = 0;

    virtual int imageHeight() const = 0;

    virtual int imageWidth() const = 0;

    virtual bool hasPixelData() const = 0;
};

#endif // DicomUtil_H
