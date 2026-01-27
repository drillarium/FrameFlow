#include "previewscenewidget.h"
#include <QPainter>
#include <QLinearGradient>

PreviewSceneWidget::PreviewSceneWidget(QWidget *_parent)
:QWidget(_parent)
{
  setAttribute(Qt::WA_OpaquePaintEvent);
  setAttribute(Qt::WA_NoSystemBackground);
}

PreviewSceneWidget::~PreviewSceneWidget()
{

}

void PreviewSceneWidget::paintEvent(QPaintEvent*)
{
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);

  const QRect r = rect();
  const QPoint center = r.center();

  /* ===== Background gradient ===== */
  QLinearGradient bg(r.topLeft(), r.bottomRight());
  bg.setColorAt(0.0, QColor(25, 30, 36));
  bg.setColorAt(1.0, QColor(15, 18, 22));
  p.fillRect(r, bg);

  /* ===== Grid ===== */
  const int gridSize = 40;
  QPen gridPen(QColor(255, 255, 255, 18));
  gridPen.setWidth(1);
  p.setPen(gridPen);

  for(int x = 0; x < r.width(); x += gridSize)
    p.drawLine(x, 0, x, r.height());

  for(int y = 0; y < r.height(); y += gridSize)
    p.drawLine(0, y, r.width(), y);

  /* ===== Center cross ===== */
  QPen centerPen(QColor(255, 255, 255, 40));
  centerPen.setWidth(1);
  p.setPen(centerPen);

  p.drawLine(center.x(), 0, center.x(), r.height());
  p.drawLine(0, center.y(), r.width(), center.y());

  /* ===== Target circles ===== */
  QColor accent(0, 180, 190);
  p.setPen(Qt::NoPen);

  p.setBrush(QColor(accent.red(), accent.green(), accent.blue(), 60));
  p.drawEllipse(center, 48, 48);

  p.setBrush(QColor(accent.red(), accent.green(), accent.blue(), 120));
  p.drawEllipse(center, 28, 28);

  p.setBrush(QColor(accent.red(), accent.green(), accent.blue(), 200));
  p.drawEllipse(center, 10, 10);

  /* ===== Text ===== */
  p.setPen(QColor(170, 180, 190));
  QFont f = font();
  f.setPointSize(10);
  p.setFont(f);

  p.drawText(
    QRect(center.x() - 100, center.y() + 60, 200, 30),
    Qt::AlignCenter,
    "Preview: Main Scene"
  );

  // boder
  const int borderWidth = 2;
  const int radius = 10;
  QRect rborder = rect();
  rborder.adjust(borderWidth / 2.0, borderWidth / 2.0, -borderWidth / 2.0, -borderWidth / 2.0);

  QColor color(Qt::black);
  if(previewMode_ == EPreviewMode::PM_PREVIEW) color = QColor(0x19BDDE);
  else if(previewMode_ == EPreviewMode::PM_PROGRAM) color = QColor(Qt::red);
  QPen pen(color);
  pen.setWidth(borderWidth);
  QPainter painter(this);
  painter.setPen(pen);
  painter.setBrush(Qt::NoBrush);

  painter.drawRoundedRect(r, radius, radius);
}
