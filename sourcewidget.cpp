#include "sourcewidget.h"
#include <QStyle>

SourceWidget::SourceWidget(const Source& _source, QWidget *_parent)
:QWidget(_parent)
,source_(_source)
{
  ui.setupUi(this);

  ui.lockButton->hide();
  ui.eyeButton->hide();
  ui.menuButton->hide();
  ui.titleLabel->setText(source_.name);
}

SourceWidget::~SourceWidget()
{
}

void SourceWidget::enterEvent(QEnterEvent*)
{
  ui.lockButton->show();
  ui.eyeButton->show();
  ui.menuButton->show();
}

void SourceWidget::leaveEvent(QEvent*)
{
  ui.lockButton->hide();
  ui.eyeButton->hide();
  ui.menuButton->hide();
}

void SourceWidget::setSelected(bool _selected)
{
  QString ss = "#mainSourceWidget {\
    background: #171B22;\
    border: 2px solid %1;\
    border-radius: 8px;\
  }\
  \
  #mainSourceWidget:hover {\
    background: #1E2430;\
    border: 2px solid %1;\
    border-radius: 8px;\
  }\
  \
  #titleLabel {\
    color: white;\
  }";

  QString color = _selected? "#19BDDE" : "transparent";
  setStyleSheet(QString(ss).arg(color));
  style()->unpolish(this);
  style()->polish(this);
  update();
}
