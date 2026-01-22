#include "widgetbutton.h"
#include <QMouseEvent>
#include <QStyle>

WidgetButton::WidgetButton(QWidget *_parent)
:QWidget(_parent)
{
  setAttribute(Qt::WA_StyledBackground, true);

  setProperty("hover", false);
  setProperty("pressed", false);
}

WidgetButton::~WidgetButton()
{
}


void WidgetButton::mousePressEvent(QMouseEvent* e)
{
  if(e->button() == Qt::LeftButton)
  {
    pressed_ = true;
    setProperty("pressed", true);
    style()->polish(this);
  }
}

void WidgetButton::mouseReleaseEvent(QMouseEvent* e)
{
  if(pressed_ && rect().contains(e->pos()))
  {
    emit clicked();
  }
  pressed_ = false;
  setProperty("pressed", false);
  style()->polish(this);
}

void WidgetButton::enterEvent(QEnterEvent*)
{
  setProperty("hover", true);
  style()->polish(this);
}

void WidgetButton::leaveEvent(QEvent*)
{
  setProperty("hover", false);
  setProperty("pressed", false);
  style()->polish(this);
}