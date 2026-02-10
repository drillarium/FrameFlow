#pragma once

#include <QWidget>
#include "scenerenderer.h"

class PreviewSceneWidget  : public QWidget
{
Q_OBJECT

public:
  enum Mode { None, Move, ResizeBottomRight, ResizeBottomCenter, ResizeBottomLeft, ResizeCenterLeft, ResizeTopLeft, ResizeTopCenter, ResizeTopRight, ResizeCenterRight };
  PreviewSceneWidget(QWidget *_parent);
  ~PreviewSceneWidget();
  void setPreviewMode(EPreviewMode _mode);
  void updateSelectedSource();

protected:
  void paintEvent(QPaintEvent* event) override;
  void mousePressEvent(QMouseEvent*) override;
  void mouseMoveEvent(QMouseEvent*) override;
  void mouseReleaseEvent(QMouseEvent*) override;
  void leaveEvent(QEvent*) override;
  QRect updateRenderRect(QSize& _videoWindowSize);
  QVector<QRectF> videoRectsToWidget();
  QPoint widgetDeltaToVideoDelta(const QPoint& deltaWidget);

signals:
  void onCurrentSourceRectChange(const QRect &_r);

protected:
  EPreviewMode previewMode_ = EPreviewMode::PM_PROGRAM;
  QVector<QRect> rects_;
  int activeRectIndex_ = -1;
  QRect activeRect_;
  QPoint lastMousePos_;
  PreviewSceneWidget::Mode mode_ = PreviewSceneWidget::None;
  QImage ARGBImage_;
};
