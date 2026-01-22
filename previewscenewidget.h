#pragma once

#include <QWidget>

class PreviewSceneWidget  : public QWidget
{
Q_OBJECT

public:
  PreviewSceneWidget(QWidget *_parent);
  ~PreviewSceneWidget();

protected:
  void paintEvent(QPaintEvent* event) override;
};
