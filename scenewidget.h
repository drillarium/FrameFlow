#pragma once

#include <QWidget>
#include "ui_scenewidget.h"
#include "scene_model.h"

class SceneWidget : public QWidget
{
Q_OBJECT

public:
  SceneWidget(Scene &_scene, QWidget *parent = nullptr);
  ~SceneWidget();
  void setSelected(bool _selected);
  QUuid id() { return scene_.id; }

protected:
  void enterEvent(QEnterEvent*) override;
  void leaveEvent(QEvent*) override;

signals:
  void onDeleteScene();
  void onRenameScene();
  void onUpScene();
  void onDownScene();

private:
  Scene scene_;
  Ui::SceneWidgetClass ui;
};

