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

  QRectF bar(r.left(), r.bottom() - height, r.width(), height);

  // Color by level
  QColor color;
  if(m_db < -18)      color = QColor("#00E676"); // green
  else if(m_db < -6)  color = QColor("#FFEB3B"); // yellow
  else                 color = QColor("#F44336"); // red

  p.setBrush(color);
  p.setPen(Qt::NoPen);
  p.drawRoundedRect(bar, 4, 4);
}
