#pragma once

#include "baserenderer.h"
#include <thread>
#include <MFormats.h>
#include <atlbase.h> // CComPtr
#include <mutex>

enum EPreviewMode { PM_PREVIEW, PM_PROGRAM };
enum ECommand { CMD_NONE, CMD_START_STREAMING, CMD_STOP_STREAMING, CMD_START_RECORDING, CMD_STOP_RECORDING };

class SceneRenderer : public BaseRenderer
{
Q_OBJECT

public:
  SceneRenderer(EPreviewMode mode);
  ~SceneRenderer();

  SourceType type() override { return SourceType::EST_SCENE; }
  bool start() override;
  bool stop() override;
  bool isRunning() override { return running_; }
  bool getFrame(CComPtr<IMFFrame>& _frame) override;
  
  void update() { update_ = true; }

  void startStreaming() { nextCommand_ = ECommand::CMD_START_STREAMING; }
  void stopStreaming() { nextCommand_ = ECommand::CMD_STOP_STREAMING; }
  void startRecording() { nextCommand_ = ECommand::CMD_START_RECORDING; }
  void stopRecording() { nextCommand_ = ECommand::CMD_STOP_RECORDING; }

signals:
  void onStartStreaming();
  void onStopStreaming();
  void onStartRecording();
  void onStopRecording();

protected:
  void workerThread();
  void renderScene(CComPtr<IMFFrame> &_frame, const QUuid &_scene);
  void nextCommandThread();

protected:
  bool running_ = false;
  std::thread workerThread_;
  CComPtr<IMFFrame> lastFrame_;
  std::mutex lastFrameMutex_;
  EPreviewMode mode_ = EPreviewMode::PM_PROGRAM;
  bool update_ = false;
  ECommand nextCommand_ = ECommand::CMD_NONE;
  CComPtr<IMFWriter> streamingWriter_;        // Streaming vars
  bool streamingWriterOpened_ = false;
  std::mutex streamingWriterMutex_;
  CComPtr<IMFWriter> fileWriter_;        // Streaming vars
  bool fileWriterOpened_ = false;
  std::mutex fileWriterMutex_;
};
