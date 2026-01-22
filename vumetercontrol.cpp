#include "vumetercontrol.h"
#include <QPainter>
#include <QTimer>
#include <QRandomGenerator>

// VumeterControl
VumeterControl::VumeterControl(QWidget *_parent)
:QWidget(_parent)
{
  ui.setupUi(this);

  QTimer* timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, [=]() {   
    double value = QRandomGenerator::global()->bounded(-60, 0);
    ui.vumeterWidget->setLevelDb(value);
  });
  timer->start(200);
}

VumeterControl::~VumeterControl()
{

}
