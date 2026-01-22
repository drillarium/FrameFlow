#include "alertbutton.h"
#include <QPainter>

AlertButton::AlertButton(QWidget *parent)
:QPushButton(parent)
{

}

AlertButton::~AlertButton()
{

}

void AlertButton::setHasAlert(bool value)
{
  if(hasAlert_ == value) return;
  hasAlert_ = value;
  update();
}

void AlertButton::paintEvent(QPaintEvent* e)
{
  QPushButton::paintEvent(e);

  if(!hasAlert_) return;

  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);

  const int dotSize = 6;
  const int margin = 6;

  QPoint topRight(width() - dotSize - margin, margin);

  p.setBrush(QColor("#19BDDE"));
  p.setPen(Qt::NoPen);
  p.drawEllipse(topRight, dotSize, dotSize);
}