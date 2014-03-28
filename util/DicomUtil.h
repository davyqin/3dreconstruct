#ifndef DICOMUTIL_H
#define DICOMUTIL_H

#include <boost/shared_ptr.hpp>
#include <string>
#include <memory>

class Image;

class DicomUtil
{
public:
    DicomUtil();

    DicomUtil(const std::string& fileName);

    ~DicomUtil();

    void setFileName(const std::string& filename);

    boost::shared_ptr<unsigned char> pixel();

    boost::shared_ptr<Image> fetchImage() const;

    int pixelLength() const;

    int imageHeight() const;

    int imageWidth() const;

    bool hasPixelData() const;

private:

    void readFile();

    class Pimpl;
    std::unique_ptr<Pimpl> _pimpl;
};

#endif // DicomUtil_H
