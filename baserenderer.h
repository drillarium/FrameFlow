#pragma once

#include <QObject>
#include <MFormats.h>
#include <atlbase.h> // CComPtr
#include "source_model.h"
#include <QRect>

// BaseRenderer
class BaseRenderer : public QObject
{
Q_OBJECT

public:
  static BaseRenderer *build(SourceType type);
  virtual ~BaseRenderer() {};
  
  QUuid id() { return source_.id; }
  virtual SourceType type() = 0;
  virtual bool setSource(Source _source) { source_ = _source; return true; }
  virtual bool start() = 0;
  virtual bool stop() = 0;
  virtual bool isRunning() = 0;
  virtual bool getFrame(CComPtr<IMFFrame>& _frame) = 0;

  static QString getVideoFormatString(int w, int h, double fr);
  static M_VID_PROPS getMVideoFormat(int w, int h, double fr);
  static M_AUD_PROPS getMAudioProps() { return { 2, 48000, 16, 0 }; };
  static void getFactors(double fr, int &num, int &den);

protected:
  Source source_;
};
