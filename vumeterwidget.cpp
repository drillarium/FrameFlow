#include "vumeterwidget.h"
#include <QPainter>

// VuMeterWidget
VuMeterWidget::VuMeterWidget(QWidget* parent)
:QWidget(parent)
{

}

void VuMeterWidget::setLevelDb(float db)
{
  m_db = qBound(-60.0f, db, 0.0f);
  update();
}

void VuMeterWidget::paintEvent(QPaintEvent*)
{
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);

  QRectF r = rect().adjusted(2, 2, -2, -2);

  // Normalize dB => 0..1
  float norm = (m_db + 60.0f) / 60.0f;
  float height = r.height() * norm;

  QRectF bar(r.left() + 5, 0, 10, r.height());
  QColor backgroundColor = QColor("#21242C");
  p.setBrush(backgroundColor);
  p.setPen(Qt::NoPen);
  p.drawRoundedRect(bar, 4, 4);

  bar = QRectF(r.left() + 5, r.bottom() - height, 10, height);

  // Color by level
  QColor color;
  if(m_db < -18)      color = QColor("#00E676"); // green
  else if(m_db < -6)  color = QColor("#FFEB3B"); // yellow
  else                 color = QColor("#F44336"); // red

  p.setBrush(color);
  p.setPen(Qt::NoPen);
  p.drawRoundedRect(bar, 4, 4);

  // line
  QColor lineColor = QColor("#F44336");
  p.setBrush(lineColor);
  p.setPen(lineColor);
  p.drawLine(r.left() + 5, 10, r.left() + 15, 10);
}
