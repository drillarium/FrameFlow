#pragma once

#include "baserenderer.h"
#include <thread>
#include <MFormats.h>
#include <atlbase.h> // CComPtr
#include <mutex>

enum EPreviewMode { PM_PREVIEW, PM_PROGRAM };

class SceneRenderer : public BaseRenderer
{
public:
  SceneRenderer(EPreviewMode mode);
  ~SceneRenderer();

  SourceType type() override { return SourceType::EST_SCENE; }
  bool start() override;
  bool stop() override;
  bool isRunning() override { return running_; }
  bool getFrame(CComPtr<IMFFrame>& _frame) override;
  
  void update() { update_ = true; }

protected:
  void workerThread();
  void renderScene(CComPtr<IMFFrame> &_frame, const QUuid &_scene);

protected:
  bool running_ = false;
  std::thread workerThread_;
  CComPtr<IMFFrame> lastFrame_;
  std::mutex lastFrameMutex_;
  EPreviewMode mode_ = EPreviewMode::PM_PROGRAM;
  bool update_ = false;
};
