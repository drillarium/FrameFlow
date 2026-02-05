#pragma once

#include <QDialog>
#include "ui_renamedialog.h"

class RenameDialog : public QDialog
{
Q_OBJECT

public:
  RenameDialog(const QString &_name, QWidget *parent = nullptr);
  ~RenameDialog();

  QString newName() { return ui.lineEdit->text(); }
  
private:
  Ui::RenameDialogClass ui;
};

