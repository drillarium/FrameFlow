#include "alertbutton.h"
#include <QPainter>

AlertButton::AlertButton(QWidget *parent)
:QPushButton(parent)
{

}

AlertButton::~AlertButton()
{

}

void AlertButton::setHasAlert(bool value, ENotificationSeverity _severity)
{
  _severity = _severity;
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
  if(severity_ == ENotificationSeverity::S_INFO) p.setBrush(QColor("#19BDDE"));
  else if(severity_ == ENotificationSeverity::S_WARNING) p.setBrush(QColor("#F5A623"));
  else p.setBrush(QColor("#E74C3C"));
  p.setPen(Qt::NoPen);
  p.drawEllipse(topRight, dotSize, dotSize);
}