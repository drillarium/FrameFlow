#pragma once

#include <QWidget>
#include "ui_vumetercontrol.h"

// VuMeterWidget
class VumeterControl : public QWidget
{
    Q_OBJECT

public:
  VumeterControl(QWidget *_parent = nullptr);
  ~VumeterControl();

private:
    Ui::VumeterControlClass ui;
};
