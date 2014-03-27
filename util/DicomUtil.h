#ifndef DICOMUTIL_H
#define DICOMUTIL_H

#include <boost/shared_ptr.hpp>
#include <string>
#include <memory>

class DicomUtil
{
public:
    DicomUtil();

    ~DicomUtil();

    void setFileName(const std::string& filename);

    boost::shared_ptr<unsigned char> pixel();

    int pixelLength() const;

    int imageHeight() const;

    int imageWidth() const;

private:

    void readImage(void);

    class Pimpl;
    std::unique_ptr<Pimpl> _pimpl;
};

#endif // DicomUtil_H
