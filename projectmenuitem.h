#pragma once

#include <QWidget>
#include "ui_projectmenuitem.h"
#include "project_model.h"

class ProjectMenuItem : public QWidget
{
Q_OBJECT

public:
  ProjectMenuItem(Project&_project, QWidget *_parent = nullptr);
  ~ProjectMenuItem();

protected slots:
  void onStartEdit();
  void onEndEdit();
  void onProjectListChanged();
  void onValidateEdit();

private:
  Ui::ProjectMenuItemClass ui;
  std::optional<Project> project_;
};

