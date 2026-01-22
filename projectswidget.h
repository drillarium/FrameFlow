#pragma once

#include <QDialog>
#include "ui_projectswidget.h"

class ProjectsWidget : public QDialog
{
Q_OBJECT

public:
  ProjectsWidget(QWidget *_parent = nullptr);
  ~ProjectsWidget();

protected:
  void updateSizeFromList();
  bool eventFilter(QObject* obj, QEvent* event) override;

protected slots:
  void onNewProject();

private:
  Ui::ProjectsWidgetClass ui;
};

