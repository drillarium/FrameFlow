#pragma once

#include <QObject>
#include "scenerenderer.h"
#include <QImage>

enum EStreamingState { SS_NONE, SS_WAITING_START, SS_STREAMING, SS_WAITING_NONE };
enum ERecordingState { RS_NONE, RS_WAITING_START, RS_RECORDING, RS_WAITING_NONE };

class RendererManager : public QObject
{
Q_OBJECT

public:
  static RendererManager& instance();
  bool reloadProjec();
  bool unload();
  bool getFrame(QUuid sourceId, CComPtr<IMFFrame> &_frame);
  EStreamingState stremingState() { return streaming_; }
  bool startStreaming();
  bool stopStreaming();
  ERecordingState recordingState() { return recording_; }
  bool startRecording();
  bool stopRecording();

signals:
  void onNewPreviewImage(QImage image);
  void onNewProgramImage(QImage image);
  void onStreamingStateChange(EStreamingState _state);
  void onRecordingStateChange(ERecordingState _state);

protected slots:
  void onTimeout();
  void onStartStreaming();
  void onStopStreaming();
  void onStartRecording();
  void onStopRecording();

private:
  explicit RendererManager(QObject* parent = nullptr);
  ~RendererManager() = default;

  Q_DISABLE_COPY_MOVE(RendererManager)

protected:
  SceneRenderer previewRenderer_;    // preview
  SceneRenderer programRenderer_;    // program
  QList<BaseRenderer *> renderers_;  // renderers involved in current project
  QUuid currentProjectUid_;
  EStreamingState streaming_ = EStreamingState::SS_NONE;
  ERecordingState recording_ = ERecordingState::RS_NONE;
};

