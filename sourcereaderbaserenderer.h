#pragma once

#include "baserenderer.h"
#include <thread>
#include <mutex>

class SourceReaderBaseRenderer : public BaseRenderer
{
public:
  SourceReaderBaseRenderer(SourceType _type);
  ~SourceReaderBaseRenderer();

  SourceType type() override { return type_; }
  bool start() override;
  bool stop() override;
  bool isRunning() override { return running_; }
  bool getFrame(CComPtr<IMFFrame>& _frame, QRect& _rect) override;
  bool setSource(Source _source);

protected:
  void workerThread();

protected:
  SourceType type_;
  bool running_ = false;
  std::thread workerThread_;
  CComPtr<IMFFrame> lastFrame_;
  std::mutex lastFrameMutex_;
  std::string url_;
  bool loop_ = true;
};
