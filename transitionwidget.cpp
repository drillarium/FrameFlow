#include "transitionwidget.h"
#include <QStyle>

TransitionWidget::TransitionWidget(const Transition& _transition, QWidget *parent)
:QWidget(parent)
{
  ui.setupUi(this);
  setTransition(_transition);
}

TransitionWidget::~TransitionWidget()
{

}

void TransitionWidget::setTransition(const Transition& _transition)
{
  transition_ = _transition;
  ui.titleLabel->setText(transition_.name);
}

void TransitionWidget::setSelected(bool _selected)
{
  QString ss = "#mainTransitionWidget {\
    background: #171B22;\
    border: 2px solid %1;\
    border-radius: 8px;\
  }\
  \
  #mainTransitionWidget:hover {\
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
