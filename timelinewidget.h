#pragma once

#include <QWidget>

struct TimelineItem
{
  double startSec;
  double durationSec;
  QString label;
};

class TimelineWidget : public QWidget
{
Q_OBJECT

public:
  TimelineWidget(QWidget *parent);
  ~TimelineWidget();

  void setZoom(int _zoom);

protected:
  void paintEvent(QPaintEvent*) override;
  void wheelEvent(QWheelEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;

  void drawHeader(QPainter& p);
  void drawCenter(QPainter& p);
  void drawFooter(QPainter& p);

  double timeToPixel(double seconds) const;
  bool isInHeader(const QPoint& pos) const { return pos.y() <= headerHeight_; }
  int hitTestItem(const QPoint& pos) const;

protected:
  QVector<TimelineItem> items_;
  double pixelsPerSecond_ = 100.0;   // zoom
  double offsetSecs_ = 0.0;          // scroll offset
  int headerHeight_ = 25;
  int footerHeight_ = 20;
  bool isPanning_ = false;
  int lastMouseX_ = 0;
  int selectedIndex_ = -1;
};
