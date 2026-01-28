#pragma once

#include <QDialog>
#include "ui_selectsourcedialog.h"

class SelectSourceDialog : public QDialog
{
    Q_OBJECT

public:
    SelectSourceDialog(QWidget *parent = nullptr);
    ~SelectSourceDialog();

private:
    Ui::SelectSourceDialogClass ui;
};

