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

    unsigned char* convertTo8Bit(unsigned char* _pData, unsigned long nNumPixels,
                                 bool bIsSigned, short nHighBit,
                                 float fRescaleSlope, float fRescaleIntercept,
                                 float fWindowCenter, float fWindowWidth);

    class Pimpl;
    std::unique_ptr<Pimpl> _pimpl;
};

#endif // DicomUtil_H
