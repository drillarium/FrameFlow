#pragma once

#include <QWidget>

enum EPreviewMode { PM_DIRECT, PM_PREVIEW, PM_PROGRAM };

class PreviewSceneWidget  : public QWidget
{
Q_OBJECT

public:
  PreviewSceneWidget(QWidget *_parent);
  ~PreviewSceneWidget();
  void setPreviewMode(EPreviewMode _mode) { previewMode_ = _mode; }

protected:
  void paintEvent(QPaintEvent* event) override;

protected:
  EPreviewMode previewMode_ = EPreviewMode::PM_DIRECT;
};
