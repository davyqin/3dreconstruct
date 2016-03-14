#pragma once

#include <boost/exception/exception.hpp>
#include <exception>

class Exception : public std::exception, public boost::exception
{
public:
  Exception();
  virtual ~Exception();
};

