#include "effectwidget.h"
#include <QStyle>

EffectWidget::EffectWidget(QWidget *parent)
:QWidget(parent)
{
  ui.setupUi(this);

  ui.editButton->hide();
  ui.cloneButton->hide();
  ui.deleteButton->hide();
}

EffectWidget::~EffectWidget()
{

}

void EffectWidget::setSelected(bool _selected)
{
  QString ss = "#mainEffectWidget {\
    background: #171B22;\
    border: 2px solid %1;\
    border-radius: 8px;\
  }\
  \
  #mainEffectWidget:hover {\
    background: #1E2430;\
    border: 2px solid %1;\
    border-radius: 8px;\
  }\
  \
  #titleLabel {\
    color: white;\
  }";

  QString color = _selected ? "#19BDDE" : "transparent";
  setStyleSheet(QString(ss).arg(color));
  style()->unpolish(this);
  style()->polish(this);
  update();
}

void EffectWidget::enterEvent(QEnterEvent* e)
{
  ui.editButton->show();
  ui.cloneButton->show();
  ui.deleteButton->show();
}

void EffectWidget::leaveEvent(QEvent* e)
{
  ui.editButton->hide();
  ui.cloneButton->hide();
  ui.deleteButton->hide();
}
