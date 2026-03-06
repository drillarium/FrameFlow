#include "timelinewidget.h"
#include <QPainter>
#include <QTime>
#include <QWheelEvent>
#include <QPainterPath>

TimelineWidget::TimelineWidget(QWidget *parent)
:QWidget(parent)
{
  TimelineItem item1 = { 0, 100, "Scene1" };
  TimelineItem item2 = { 100, 50, "Scene2" };
  TimelineItem item3 = { 150, 5, "Scene3" };
  items_.push_back(item1);
  items_.push_back(item2);
  items_.push_back(item3);

  setMouseTracking(true);
}

TimelineWidget::~TimelineWidget()
{

}

double TimelineWidget::timeToPixel(double sec) const
{
  return (sec - offsetSecs_) * pixelsPerSecond_;
}

void TimelineWidget::paintEvent(QPaintEvent*)
{
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);

  p.fillRect(rect(), Qt::transparent);

  drawHeader(p);
  drawCenter(p);
  drawFooter(p);
  drawPointer(p);
}

void TimelineWidget::drawHeader(QPainter& p)
{
  QRect r(0, 0, width(), headerHeight_);
  p.fillRect(r, Qt::transparent);

  double secondsPerMajorTick;

  if(pixelsPerSecond_ > 300)
    secondsPerMajorTick = 1.;
  else if(pixelsPerSecond_ > 120)
    secondsPerMajorTick = 2.5;
  else if(pixelsPerSecond_ > 60)
    secondsPerMajorTick = 5.0;
  else if(pixelsPerSecond_ > 20)
    secondsPerMajorTick = 5.0;
  else
    secondsPerMajorTick = 10.0;

  double startTime = offsetSecs_;
  double endTime = offsetSecs_ + width() / pixelsPerSecond_;

  int firstTick = std::floor(startTime / secondsPerMajorTick);

  p.setPen(QColor("#697485"));

  for(int i = firstTick; ; ++i)
  {
    double t = i * secondsPerMajorTick;
    if(t > endTime)
      break;

    int x = timeToPixel(t);

    // p.drawLine(x, r.bottom() - 10, x, r.bottom());

    QString label = QTime::fromMSecsSinceStartOfDay(t * 1000)
      .toString("hh:mm:ss");

    p.drawText(x + 3, r.bottom() - 15, label);
  }
}

void TimelineWidget::drawFooter(QPainter& p)
{
  QRect r(0,
    height() - footerHeight_,
    width(),
    footerHeight_);

  p.fillRect(r, Qt::transparent);

  p.setPen(QColor("#697485"));
  p.drawText(r.adjusted(10, 0, -10, 0),
    Qt::AlignVCenter | Qt::AlignLeft,
    QString("%1 items").arg(items_.size()));
}

void TimelineWidget::wheelEvent(QWheelEvent* event)
{
  if(event->angleDelta().y() > 0)
    pixelsPerSecond_ *= 1.15;
  else
    pixelsPerSecond_ /= 1.15;

  pixelsPerSecond_ = std::clamp(pixelsPerSecond_, 5.0, 2000.0);

  update();
}

QString formatDuration(double seconds)
{
  if(seconds < 10.0)
    return QString::number(seconds, 'f', 2) + "s";
  else
    return QString::number(seconds, 'f', 1) + "s";
}

void TimelineWidget::drawCenter(QPainter& p)
{
  QRect r(0,
    headerHeight_,
    width(),
    height() - headerHeight_ - footerHeight_);

  p.save();

  p.setRenderHint(QPainter::Antialiasing, true);

  QPainterPath bgPath;
  bgPath.addRoundedRect(r.adjusted(5, 5, -5, -5), 12, 12);

  p.fillPath(bgPath, QColor("#1E232A"));

  // Clip to rounded background
  p.setClipPath(bgPath);

  // Now draw items inside clipped region
  for(int i = 0; i < items_.size(); ++i)
  {
    const auto& item = items_[i];
    int x = timeToPixel(item.startSec);
    int w = item.durationSec * pixelsPerSecond_;

    QRect itemRect(x,
      r.top() + 10,
      w,
      r.height() - 20);

    QColor baseColor = QColor("#272C35");

    if(i == selectedIndex_)
    {
      baseColor = QColor("#19D5E6");
      p.setPen(QPen(Qt::black, 2));
    }
    else
    {
      p.setPen(QPen(Qt::white, 2));
    }

    p.setBrush(baseColor);    

    QPainterPath itemPath;
    itemPath.addRoundedRect(itemRect, 8, 8);

    p.fillPath(itemPath, p.brush());
    // p.strokePath(itemPath, p.pen());

    const int padding = 8;

    QString durationText = formatDuration(item.durationSec);
    QFontMetrics fm(p.font());

    // Measure duration text width
    int durationWidth = fm.horizontalAdvance(durationText);

    // Define right text rect
    QRect durationRect = itemRect.adjusted(
      padding,
      0,
      -padding,
      0
    );

    durationRect.setLeft(itemRect.right() - durationWidth - padding);

    QRect labelRect = itemRect.adjusted(
      padding,
      0,
      -durationWidth - 2 * padding,
      0
    );

    // Left: label (elided if needed)
    QString elidedLabel = fm.elidedText(
      item.label,
      Qt::ElideRight,
      labelRect.width()
    );

    p.drawText(itemRect.adjusted(5, 0, -5, 0),
      Qt::AlignVCenter | Qt::AlignLeft,
      elidedLabel);

    // Right: duration
    p.drawText(durationRect,
      Qt::AlignVCenter | Qt::AlignRight,
      durationText);
  }

  p.restore();
}

bool TimelineWidget::isInPointer(const QPoint& pos)
{
  int x = timeToPixel(positionSecs_);
  QRect centerRect(x-5, 0, 10, height());

  return centerRect.contains(pos);
}

void TimelineWidget::mousePressEvent(QMouseEvent* event)
{
  if(event->button() == Qt::LeftButton)
  {
    if(isInPointer(event->pos()))
    {
      isMovingPointer_ = true;
      lastMouseX_ = event->pos().x();
      setCursor(Qt::ClosedHandCursor);
    }
    else if(isInHeader(event->pos()))
    {
      isPanning_ = true;
      lastMouseX_ = event->pos().x();
      setCursor(Qt::ClosedHandCursor);
    }    
    else {   
      int index = hitTestItem(event->pos());
      if(index != -1)
      {
        if(selectedIndex_ != index)
        {
          selectedIndex_ = index;
          update();
        }
      }
      else
      {
        // Clicked empty area => clear selection
        if(selectedIndex_ != -1)
        {
          selectedIndex_ = -1;
          update();
        }
      }
    }
  }

  QWidget::mousePressEvent(event);
}

void TimelineWidget::mouseMoveEvent(QMouseEvent* event)
{
  if(isPanning_)
  {
    int dx = event->pos().x() - lastMouseX_;

    // Convert pixels to seconds
    double deltaSec = dx / pixelsPerSecond_;

    offsetSecs_ -= deltaSec;   // subtract for natural direction

    if(offsetSecs_ < 0)
      offsetSecs_ = 0;

    lastMouseX_ = event->pos().x();

    update();
    return;
  }

  if(isMovingPointer_)
  {
    int dx = event->pos().x() - lastMouseX_;

    // Convert pixels to seconds
    double deltaSec = dx / pixelsPerSecond_;

    positionSecs_ += deltaSec;
    if(positionSecs_ < 0)
      positionSecs_ = 0;

    lastMouseX_ = event->pos().x();

    update();
    return;
  }

  if(isInPointer(event->pos()))
    setCursor(Qt::OpenHandCursor);
  else if(hitTestItem(event->pos()) >= 0)
    setCursor(Qt::PointingHandCursor);
  else if(isInHeader(event->pos()))
    setCursor(Qt::OpenHandCursor);
  else
    unsetCursor();
}

void TimelineWidget::mouseReleaseEvent(QMouseEvent* event)
{
  if(event->button() == Qt::LeftButton)
  {
    if(isPanning_)
    {
      isPanning_ = false;
      setCursor(Qt::ArrowCursor);
    }
    if(isMovingPointer_)
    {
      isMovingPointer_ = false;
      setCursor(Qt::ArrowCursor);
    }
  }

  QWidget::mouseReleaseEvent(event);
}

int TimelineWidget::hitTestItem(const QPoint& pos) const
{
  QRect centerRect(0,
    headerHeight_,
    width(),
    height() - headerHeight_ - footerHeight_);

  if(!centerRect.contains(pos))
    return -1;

  for(int i = 0; i < items_.size(); ++i)
  {
    const auto& item = items_[i];

    int x = timeToPixel(item.startSec);
    int w = item.durationSec * pixelsPerSecond_;

    QRect itemRect(x,
      centerRect.top() + 10,
      w,
      centerRect.height() - 20);

    if(itemRect.contains(pos))
      return i;
  }

  return -1;
}

void TimelineWidget::drawPointer(QPainter& p)
{
  int x = timeToPixel(positionSecs_);
  p.setPen(QPen(Qt::red, 2));
  p.drawLine(x, 0, x, height());
}
