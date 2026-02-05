#pragma once

#include <QDialog>
#include "ui_createscenedialog.h"

class CreateSceneDialog : public QDialog
{
Q_OBJECT

public:
  CreateSceneDialog(QWidget *parent = nullptr);
  ~CreateSceneDialog();

  QString name() { return ui.lineEdit->text(); }

private:
  Ui::CreateSceneDialogClass ui;
};

