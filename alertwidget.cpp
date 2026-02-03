#include "alertwidget.h"
#include <QStyle>

AlertWidget::AlertWidget(QWidget *parent)
:QWidget(parent)
{
  ui.setupUi(this);
  ui.closeAlertButton->hide();
  connect(ui.closeAlertButton, &QPushButton::clicked, this, &AlertWidget::onRemoveAlert);
}

AlertWidget::~AlertWidget()
{

}

void AlertWidget::enterEvent(QEnterEvent* event)
{
  ui.closeAlertButton->show();
  QWidget::enterEvent(event);
}

void AlertWidget::leaveEvent(QEvent* event)
{
  ui.closeAlertButton->hide();
  QWidget::leaveEvent(event);
}

void AlertWidget::setSelected(bool _selected)
{
  QString ss = "#mainAlertWidget {\
  background: #1C1F26;\
  border: 1px solid %1;\
  border-radius: 12px;\
}\
\
QLabel {\
  color: grey;\
}\
\
#titleLabel {\
  color: white;\
}\
\
#closeAlertButton {\
  border: 0px solid black;\
}";

  QString color = _selected ? "white" : "transparent";
  setStyleSheet(QString(ss).arg(color));
  style()->unpolish(this);
  style()->polish(this);
  update();

}
