#pragma once

#include <MFormats.h>
#include <atlbase.h> // CComPtr

// BaseRenderer
class BaseRenderer
{
protected:
  static M_VID_PROPS getMVideoFormat(int w, int h, double fr);
  static M_AUD_PROPS getMAudioProps() { return { 2, 48000, 16, 0 }; };
  static void getFactors(double fr, int &num, int &den);
};
