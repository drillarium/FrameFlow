#pragma once

#include <QDialog>
#include "ui_selectsourcedialog.h"

class SelectSourceDialog : public QDialog
{
Q_OBJECT

public:
  SelectSourceDialog(QWidget *parent = nullptr);
  ~SelectSourceDialog();

  Source source() { return source_; }

protected slots:
  void onAccept();
  void onItemSelectedChange();

private:
  Ui::SelectSourceDialogClass ui;
  Source source_;
};

