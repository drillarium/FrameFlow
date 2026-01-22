#pragma once

#include <QWidget>
#include "ui_projectmenuitem.h"

class ProjectMenuItem : public QWidget
{
Q_OBJECT

public:
  ProjectMenuItem(QWidget *_parent = nullptr);
  ~ProjectMenuItem();

protected slots:
  void onStartEdit();
  void onEndEdit();

private:
  Ui::ProjectMenuItemClass ui;
};

