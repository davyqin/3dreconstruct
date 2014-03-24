#pragma once

#include <memory>

class Image
{
public:
    Image();

    ~Image();

    // int pixelLength() const;

    // int imageHeight() const;

    // int imageWidth() const;

private:

    class Pimpl;

    std::unique_ptr<Pimpl> _pimpl;


};
