#ifndef QZJDICOMUTIL_H
#define QZJDICOMUTIL_H

#include "DicomUtil.h"

#include <boost/shared_ptr.hpp>
#include <string>
#include <memory>

class Image;

class QZJDicomUtil : public DicomUtil
{
public:
    QZJDicomUtil();

    QZJDicomUtil(const std::string& fileName);

    ~QZJDicomUtil();

    void setFileName(const std::string& filename);

    boost::shared_ptr<unsigned short> pixel();

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

#endif // QZJDicomUtil_H
