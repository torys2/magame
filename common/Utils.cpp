#include "StdAfx.h"

#include "Utils.h"

std::string IntToStr(int number) {
    return boost::lexical_cast<std::string>(number);
}

int StrToInt(const std::string& number) {
  return boost::lexical_cast<int>(number);
}
