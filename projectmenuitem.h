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

  void setCurrentProject(QUuid _current);
  QUuid id() { return project_? project_->id : QUuid(); }

protected slots:
  void onStartEdit();
  void onEndEdit();
  void onProjectListChanged();
  void onValidateEdit();
  void onDeleteProject();

protected:
  bool eventFilter(QObject* obj, QEvent* event) override;

signals:
  void itemClicked();

private:
  Ui::ProjectMenuItemClass ui;
  std::optional<Project> project_;

public:
  static bool deletingItem_;
};

