#pragma once

#include <QDialog>
#include "ui_editsourcedialog.h"
#include "source_model.h"

class EditSourceDialog : public QDialog
{
Q_OBJECT

public:
  EditSourceDialog(const Source& _source, QWidget *parent = nullptr);
  ~EditSourceDialog();

  Source newSource() { return source_; }

protected slots:
  void onAccept();

private:
  Ui::EditSourceDialogClass ui; 
  Source source_;
};

