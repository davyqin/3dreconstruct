#ifndef DICOMUTIL_H
#define DICOMUTIL_H

#include <string>

class DicomUtil
{
public:
    DicomUtil();

    DicomUtil(const std::string& filename);

    ~DicomUtil();

    void setFileName(const std::string& filename);

    unsigned char* pixel();

    int pixelLength() const;

    int imageHeight() const;

    int imageWidth() const;

private:

    void readImage(void);

    unsigned char* convertTo8Bit(unsigned char* _pData, unsigned long nNumPixels,
                                 bool bIsSigned, short nHighBit,
                                 float fRescaleSlope, float fRescaleIntercept,
                                 float fWindowCenter, float fWindowWidth);

    std::string _fileName;
    unsigned char* _pData;
    unsigned char* _pDataOld;
};

#endif // DicomUtil_H
