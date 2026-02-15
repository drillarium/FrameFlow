#pragma once

#include "baserenderer.h"
#include <thread>
#include <mutex>
#include <QColor>

class TextRenderer : public BaseRenderer
{
public:
  TextRenderer();
  ~TextRenderer();

  SourceType type() override { return SourceType::EST_TEXT; }
  bool start() override;
  bool stop() override;
  bool isRunning() override { return running_; }
  bool getFrame(CComPtr<IMFFrame>& _frame) override;
  bool setSource(Source _source);

protected:
  void workerThread();

protected:
  bool running_ = false;
  std::thread workerThread_;
  CComPtr<IMFFrame> lastFrame_;
  std::mutex lastFrameMutex_;
  QColor color_;
  QString text_;
  QString font_;
};
