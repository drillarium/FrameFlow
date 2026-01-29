#pragma once

#include <QDialog>
#include "ui_projectswidget.h"

class ProjectsWidget : public QDialog
{
Q_OBJECT

public:
  ProjectsWidget(QWidget *_parent = nullptr);
  ~ProjectsWidget();

  void updateProjectList();
  void setCurrentProject(QUuid uid);

protected:
  void updateSizeFromList();
  bool eventFilter(QObject* obj, QEvent* event) override;
  void switchToProject(QUuid& _project);

protected slots:
  void onNewProject();

private:
  Ui::ProjectsWidgetClass ui;
  bool switchToProject_ = false;
};

