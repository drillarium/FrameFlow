#pragma once

#include <QDialog>
#include "ui_effectsdialog.h"

class EffectsDialog : public QDialog
{
    Q_OBJECT

public:
    EffectsDialog(QWidget *parent = nullptr);
    ~EffectsDialog();

private:
    Ui::EffectsDialogClass ui;
};

