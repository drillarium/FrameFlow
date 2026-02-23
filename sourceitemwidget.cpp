#include "sourceitemwidget.h"
#include <QStyle>

SourceItemWidget::SourceItemWidget(SourceType type, QWidget *parent)
:QWidget(parent)
,type_(type)
{
  ui.setupUi(this);
  ui.titleLabel->setText(sourceTitle(type));
  ui.descLabel->setText(sourceDescription(type));
}

SourceItemWidget::~SourceItemWidget()
{

}

void SourceItemWidget::setSelected(bool _selected)
{
  QString ss = "#mainSourceItemWidget {\
  background: #171B22;\
  border: 1px solid %1;\
  border-radius: 4px;\
}\
\
#mainSourceItemWidget:hover {\
  background: #303541;\
  border: 1px solid %2;\
  border-radius: 4px;\
}\
\
#titleLabel {\
  color: white;\
}\
\
#descLabel {\
  color: #7B899D;\
}";

  QString color = _selected ? "#19BDDE" : "transparent";
  QString color2 = _selected ? "#19BDDE" : "#7B899D";
  setStyleSheet(QString(ss).arg(color).arg(color2));
  style()->unpolish(this);
  style()->polish(this);
  update();
}
