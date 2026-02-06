#pragma once

#include <QObject>
#include "scenerenderer.h"
#include <QImage>

class RendererManager : public QObject
{
Q_OBJECT

public:
  static RendererManager& instance();
  bool reloadProjec();
  bool unload();
  bool getFrame(QUuid sourceId, CComPtr<IMFFrame> &_frame);

signals:
  void onNewPreviewImage(QImage image);
  void onNewProgramImage(QImage image);

protected slots:
  void onTimeout();

private:
  explicit RendererManager(QObject* parent = nullptr);
  ~RendererManager() = default;

  Q_DISABLE_COPY_MOVE(RendererManager)

protected:
  SceneRenderer previewRenderer_;    // preview
  SceneRenderer programRenderer_;    // program
  QList<BaseRenderer *> renderers_;  // renderers involved in current project
};

