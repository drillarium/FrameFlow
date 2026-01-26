#pragma once

#include <QWidget>
#include "ui_scenewidget.h"

class SceneWidget : public QWidget
{
Q_OBJECT

public:
  SceneWidget(QWidget *parent = nullptr);
  ~SceneWidget();
  void setSelected(bool _selected);

protected:
  void enterEvent(QEnterEvent*) override;
  void leaveEvent(QEvent*) override;

private:
  Ui::SceneWidgetClass ui;
};

