#pragma once

#include <QWidget>
#include "scenerenderer.h"

enum EPreviewMode { PM_DIRECT, PM_PREVIEW, PM_PROGRAM };

class PreviewSceneWidget  : public QWidget
{
Q_OBJECT

public:
  enum Mode { None, Move, Resize };
  PreviewSceneWidget(QWidget *_parent);
  ~PreviewSceneWidget();
  void setPreviewMode(EPreviewMode _mode) { previewMode_ = _mode; }

protected:
  void paintEvent(QPaintEvent* event) override;
  void mousePressEvent(QMouseEvent*) override;
  void mouseMoveEvent(QMouseEvent*) override;
  void mouseReleaseEvent(QMouseEvent*) override;
  void leaveEvent(QEvent*) override;
  QRect updateRenderRect();

protected:
  EPreviewMode previewMode_ = EPreviewMode::PM_DIRECT;
  QVector<QRect> rects_;
  int activeRectIndex_ = -1;
  QPoint lastMousePos_;
  Mode mode_ = None;
  SceneRenderer *senceneRenderer_ = nullptr;
  QSize videoWindowSize_ { 1920, 1080 };
  QImage ARGBImage_;
};
