#pragma once

#include <QDialog>
#include "ui_newprojectdialog.h"
#include "project_manager.h"

class NewProjectDialog : public QDialog
{
  Q_OBJECT

public:
  NewProjectDialog(QWidget *parent = nullptr);
  ~NewProjectDialog();

  Project project() { return project_; }

protected slots:
  void onCreateProject();
  void onSelectionChange();

private:
  Ui::NewProjectDialogClass ui;
  Project project_;
};

