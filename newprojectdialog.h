#pragma once

#include <QDialog>
#include "ui_newprojectdialog.h"

class NewProjectDialog : public QDialog
{
  Q_OBJECT

public:
  NewProjectDialog(QWidget *parent = nullptr);
  ~NewProjectDialog();

private:
  Ui::NewProjectDialogClass ui;
};

