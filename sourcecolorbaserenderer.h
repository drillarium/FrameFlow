#pragma once

#include "baserenderer.h"
#include <thread>
#include <string>
#include <mutex>

// SourceColorBaseRenderer
class SourceColorBaseRenderer : public BaseRenderer
{
public:
  SourceColorBaseRenderer();
  ~SourceColorBaseRenderer();

  SourceType type() override { return SourceType::EST_COLOR; }
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
  std::string colorParams_ = "solid_color = 'FF00FF(255)'";
  CComPtr<IMFFrame> lastFrame_;
  std::mutex lastFrameMutex_;
};