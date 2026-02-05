#pragma once

#include <QObject>
#include "scenerenderer.h"

class RendererManager : public QObject
{
Q_OBJECT

public:
  static RendererManager& instance();

private:
  explicit RendererManager(QObject* parent = nullptr);
  ~RendererManager() = default;

  Q_DISABLE_COPY_MOVE(RendererManager)

protected:
  SceneRenderer previewRenderer_;    // preview
  SceneRenderer programRenderer_;    // program
  QList<BaseRenderer *> renderers_;  // renderers involved in current project
};

