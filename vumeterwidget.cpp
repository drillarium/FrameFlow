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

  p.fillRect(rect(), Qt::transparent);

  // header
  drawHeader(p);
  drawBar(p);
}

void VuMeterWidget::drawBar(QPainter& _p)
{
  QRect r(0, headerHeight_, width(), height() - headerHeight_);

  r = r.adjusted(2, 2, -2, -2);

  // Normalize dB => 0..1
  float norm = (m_db + 60.0f) / 60.0f;
  float width = r.width() * norm;

  QRectF bar(r);
  QColor backgroundColor = QColor("#1E293B");
  _p.setBrush(backgroundColor);
  _p.setPen(Qt::NoPen);
  _p.drawRoundedRect(bar, 4, 4);

  // Color by level
  QColor color;
  if(m_db < -18)      color = QColor("#00E676"); // green
  else if(m_db < -6)  color = QColor("#FFEB3B"); // yellow
  else                color = QColor("#F44336"); // red

  bar = QRectF(r.left(), r.top(), width, r.height());

  _p.setBrush(color);
  _p.setPen(Qt::NoPen);
  _p.drawRoundedRect(bar, 4, 4);

  // line
  QColor lineColor = QColor("#F44336");
  _p.setBrush(lineColor);
  _p.setPen(lineColor);
  _p.drawLine(r.right() - 15, r.top(), r.right() - 15, r.bottom());
}

void VuMeterWidget::drawHeader(QPainter &_p)
{
  QRect r(0, 0, width(), headerHeight_);
  _p.fillRect(r, Qt::transparent);

  double dist = width() / 4;
  
  QStringList sl = QStringList() << "-60" << "-30" << "-18" << "-6" << "0BD";

  QFont font;
  font.setPointSize(7);
  _p.setFont(font);

  for(int i = 0; i < 5; ++i)
  {
    if(i < 4) _p.setPen(QColor("#94A3B8"));
    else _p.setPen(QColor("#C44444"));
    int x = dist * i;
    if(i == 4) x -= 20;
    QString label = sl[i];
    _p.drawText(x, r.bottom(), sl[i]);
  }
}
