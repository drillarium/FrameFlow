#pragma once

#include "sourcerenderer.h"
#include <thread>
#include <string>

// SourceColorBaseRenderer
class SourceColorBaseRenderer : public SourceRenderer
{
public:
  SourceColorBaseRenderer();
  ~SourceColorBaseRenderer();

  bool start();
  bool stop();
  bool isRunning() { return running_; }

protected:
  void workerThread();

protected:
  bool running_ = false;
  std::thread workerThread_;
  std::string colorParams_ = "solid_color = 'FF00FF(255)'";
};