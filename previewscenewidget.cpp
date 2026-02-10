#include "previewscenewidget.h"
#include <QPainter>
#include <QLinearGradient>
#include <QMouseEvent>
#include "project_manager.h"
#include "renderermanager.h"

PreviewSceneWidget::PreviewSceneWidget(QWidget *_parent)
:QWidget(_parent)
{
  setAttribute(Qt::WA_OpaquePaintEvent);
  setAttribute(Qt::WA_NoSystemBackground);

  setMouseTracking(true);
}

PreviewSceneWidget::~PreviewSceneWidget()
{

}

void PreviewSceneWidget::updateSelectedSource()
{
  rects_.clear();

  bool found = false;
  ProjectManager& pm = ProjectManager::instance();
  auto currentProject = pm.currentProject();
  if(currentProject)
  {
    QUuid currentSceneId = pm.currentSceneId();
    for(int i = 0; i < currentProject->scenes.size() && !found; ++i)
    {
      Scene scene = currentProject->scenes[i];
      if(scene.id == currentSceneId)
      {
        QUuid currentSourceId = pm.currentSourceId();
        for(int j = 0; j < scene.sources.size() && !found; ++j)
        {
          Source source = scene.sources[j];
          found = source.id == currentSourceId;
          if(found)
          {
            QRect r = getSourceRect(source);
            rects_.push_back(r);
          }
        }
      }
    }
  }

  update();
}

void PreviewSceneWidget::setPreviewMode(EPreviewMode _mode)
{
  RendererManager &rm = RendererManager::instance();

  previewMode_ = _mode;
  if(previewMode_ == EPreviewMode::PM_PREVIEW)
  {
    connect(&rm, &RendererManager::onNewPreviewImage, this, [&](QImage image) {
      ARGBImage_ = image.copy();
      update();
    });
  }
  else if(previewMode_ == EPreviewMode::PM_PROGRAM)
  {
    connect(&rm, &RendererManager::onNewProgramImage, this, [&](QImage image) {
      ARGBImage_ = image.copy();
      update();
    });
  }
}

QRect PreviewSceneWidget::updateRenderRect(QSize &_videoWindowSize)
{
  if(width() <= 0 || height() <= 0) return QRect();

  ProjectManager &pm = ProjectManager::instance();
  auto currentProject = pm.currentProject();
  if(!currentProject) return QRect();
  _videoWindowSize = { currentProject->width, currentProject->height };

  const double sx = double(width()) / _videoWindowSize.width();
  const double sy = double(height()) / _videoWindowSize.height();
  const double scale = (std::min)(sx, sy);

  const QSize fitted(int(_videoWindowSize.width() * scale), int(_videoWindowSize.height() * scale));
  const QPoint topLeft((width() - fitted.width()) / 2, (height() - fitted.height()) / 2);
  return QRect(topLeft, fitted);
}

int findRectAt(const QVector<QRectF>& rects, const QPoint& pos)
{
  for(int i = rects.size() - 1; i >= 0; --i)
  {
    if(rects[i].contains(pos))
    {
      return i;
    }
  }
  return -1;
}

static Qt::CursorShape cursorForMode(PreviewSceneWidget::Mode mode)
{
  switch(mode)
  {
    case PreviewSceneWidget::Move:               return Qt::SizeAllCursor;
    case PreviewSceneWidget::ResizeBottomRight:  return Qt::SizeFDiagCursor;
    case PreviewSceneWidget::ResizeBottomCenter: return Qt::SizeVerCursor;
    case PreviewSceneWidget::ResizeBottomLeft:   return Qt::SizeBDiagCursor;
    case PreviewSceneWidget::ResizeCenterLeft:   return Qt::SizeHorCursor;
    case PreviewSceneWidget::ResizeTopLeft:      return Qt::SizeFDiagCursor;
    case PreviewSceneWidget::ResizeTopCenter:    return Qt::SizeVerCursor;
    case PreviewSceneWidget::ResizeTopRight:     return Qt::SizeBDiagCursor;
    case PreviewSceneWidget::ResizeCenterRight:  return Qt::SizeHorCursor;
    default:                                     return Qt::ArrowCursor;
  }
}

static PreviewSceneWidget::Mode modeForPosition(const QRectF &_rect, const QPoint &_position)
{
  qreal size = 10.0;

  QRectF resizeHandle(_rect.bottomRight() - QPointF(size / 2, size / 2), _rect.bottomRight() + QPointF(size / 2, size / 2));
  if(resizeHandle.contains(_position)) return PreviewSceneWidget::ResizeBottomRight;

  resizeHandle = QRectF(_rect.bottomLeft() + QPointF(size / 2, size / 2), _rect.bottomLeft() - QPointF(size / 2, size / 2));
  if(resizeHandle.contains(_position)) return PreviewSceneWidget::ResizeBottomLeft;

  resizeHandle = QRectF(_rect.topLeft() + QPointF(size / 2, size / 2), _rect.topLeft() - QPointF(size / 2, size / 2));
  if(resizeHandle.contains(_position)) return PreviewSceneWidget::ResizeTopLeft;

  resizeHandle = QRectF(_rect.topRight() - QPointF(size / 2, size / 2), _rect.topRight() + QPointF(size / 2, size / 2));
  if(resizeHandle.contains(_position)) return PreviewSceneWidget::ResizeTopRight;


  resizeHandle = QRectF(QPoint(_rect.center().x(), _rect.bottom()) - QPointF(size / 2, size / 2), QPoint(_rect.center().x(), _rect.bottom()) + QPointF(size / 2, size / 2));
  if(resizeHandle.contains(_position)) return PreviewSceneWidget::ResizeBottomCenter;

  resizeHandle = QRectF(QPoint(_rect.x(), _rect.center().y()) - QPointF(size / 2, size / 2), QPoint(_rect.x(), _rect.center().y()) + QPointF(size / 2, size / 2));
  if(resizeHandle.contains(_position)) return PreviewSceneWidget::ResizeCenterLeft;

  resizeHandle = QRectF(QPoint(_rect.center().x(), _rect.top()) - QPointF(size / 2, size / 2), QPoint(_rect.center().x(), _rect.top()) + QPointF(size / 2, size / 2));
  if(resizeHandle.contains(_position)) return PreviewSceneWidget::ResizeTopCenter;

  resizeHandle = QRectF(QPoint(_rect.x() + _rect.width(), _rect.center().y()) - QPointF(size / 2, size / 2), QPoint(_rect.x() + _rect.width(), _rect.center().y()) + QPointF(size / 2, size / 2));
  if(resizeHandle.contains(_position)) return PreviewSceneWidget::ResizeCenterRight;

  return PreviewSceneWidget::Move;
}

QRectF videoRectToWidget(const QRectF& videoRect, const QSizeF& videoSize, const QRectF& videoRectInWidget)
{
  const float sx = videoRectInWidget.width() / videoSize.width();
  const float sy = videoRectInWidget.height() / videoSize.height();

  return QRectF(videoRectInWidget.left() + videoRect.x() * sx, videoRectInWidget.top() + videoRect.y() * sy, videoRect.width() * sx, videoRect.height() * sy);
}

void PreviewSceneWidget::paintEvent(QPaintEvent *_event)
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

  p.drawText(QRect(center.x() - 100, center.y() + 60, 200, 30), Qt::AlignCenter, "FrameFlow");

  // render
  QSize videoWindowSize;
  QRect renderRect = updateRenderRect(videoWindowSize);
  if(!ARGBImage_.isNull())
  {
    p.drawImage(renderRect, ARGBImage_);
  }

  // rects
  QVector<QRectF> rects = videoRectsToWidget();
  for(int i = 0; i < rects.size(); ++i)
  {
    QPen rectPen(Qt::blue, 2);
    p.setPen(rectPen);
    p.drawRect(rects[i]);

    int x = rects_[i].x();
    int y = rects_[i].y();
    int w = rects_[i].width();
    int h = rects_[i].height();

    /* ===== Text ===== */
    QFont f = font();
    f.setPointSize(10);
    p.setFont(f);

    // padding
    QRectF rPadding = rects[i].adjusted(8, 8, -8, -8);
    p.drawText(rPadding, Qt::AlignCenter, QString("%1x%2").arg(w).arg(h));
    p.drawText(rPadding, Qt::AlignLeft | Qt::AlignVCenter, QString("%1").arg(x));
    p.drawText(rPadding, Qt::AlignTop | Qt::AlignHCenter, QString("%1").arg(y));
    p.drawText(rPadding, Qt::AlignRight | Qt::AlignVCenter, QString("%1").arg(videoWindowSize.width() - (x + w)));
    p.drawText(rPadding, Qt::AlignBottom | Qt::AlignHCenter, QString("%1").arg(videoWindowSize.height() - (y + h)));

    /* lines vertical and horizontal */
    p.drawLine(renderRect.x(), rects[i].y() + (rects[i].height() / 2), rects[i].x(), rects[i].y() + (rects[i].height() / 2));
    p.drawLine(rects[i].x() + rects[i].width(), rects[i].y() + (rects[i].height() / 2), renderRect.x() + renderRect.width(), rects[i].y() + (rects[i].height() / 2));

    p.drawLine(rects[i].x() + (rects[i].width() / 2), renderRect.y(), rects[i].x() + (rects[i].width() / 2), rects[i].y());
    p.drawLine(rects[i].x() + (rects[i].width() / 2), rects[i].y() + rects[i].height(), rects[i].x() + (rects[i].width() / 2), renderRect.y() + renderRect.height());

    QPointF topCenterCenter(rects[i].x() + (rects[i].width() / 2), rects[i].y());
    QPointF bottomCenterCenter(rects[i].x() + (rects[i].width() / 2), rects[i].y() + rects[i].height());
    QPointF leftCenterCenter(rects[i].x(), rects[i].y() + (rects[i].height() / 2));
    QPointF rightCenterCenter(rects[i].x() + rects[i].width(), rects[i].y() + (rects[i].height() / 2));
    QPointF topLeftCenter(rects[i].x(), rects[i].y());
    QPointF topRightCenter(rects[i].x() + rects[i].width(), rects[i].y());
    QPointF bottomLeftCenter(rects[i].x(), rects[i].y() + rects[i].height());
    QPointF bottomRightCenter(rects[i].x() + rects[i].width(), rects[i].y() + +rects[i].height());

    qreal size = 10.0;
    p.drawRect(QRectF(topCenterCenter.x() - size / 2, topCenterCenter.y() - size / 2, size, size));
    p.drawRect(QRectF(bottomCenterCenter.x() - size / 2, bottomCenterCenter.y() - size / 2, size, size));
    p.drawRect(QRectF(leftCenterCenter.x() - size / 2, leftCenterCenter.y() - size / 2, size, size));
    p.drawRect(QRectF(rightCenterCenter.x() - size / 2, rightCenterCenter.y() - size / 2, size, size));
    p.drawRect(QRectF(topLeftCenter.x() - size / 2, topLeftCenter.y() - size / 2, size, size));
    p.drawRect(QRectF(topRightCenter.x() - size / 2, topRightCenter.y() - size / 2, size, size));
    p.drawRect(QRectF(bottomLeftCenter.x() - size / 2, bottomLeftCenter.y() - size / 2, size, size));
    p.drawRect(QRectF(bottomRightCenter.x() - size / 2, bottomRightCenter.y() - size / 2, size, size));
  }

  // boder
  const int borderWidth = 2;
  const int radius = 0;
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

QVector<QRectF> PreviewSceneWidget::videoRectsToWidget()
{
  // normalize
  QSize videoWindowSize;
  QRect renderRect = updateRenderRect(videoWindowSize);
  QVector<QRectF> rects;
  for(const QRect& r : rects_) { rects.push_back(QRectF(r)); }

  for(QRectF& r : rects)
  {
    r = videoRectToWidget(r, videoWindowSize, renderRect);
  }

  return rects;
}

void PreviewSceneWidget::mousePressEvent(QMouseEvent* e)
{
  QVector<QRectF> rects = videoRectsToWidget();

  activeRectIndex_ = findRectAt(rects, e->pos());
  lastMousePos_ = e->pos();
  mode_ = None;

  if(activeRectIndex_ >= 0)
  {
    activeRect_ = rects_[activeRectIndex_];

    QRectF& r = rects[activeRectIndex_];    
    mode_ = modeForPosition(r, e->pos());
    setCursor(cursorForMode(mode_));
  }
}

QPoint PreviewSceneWidget::widgetDeltaToVideoDelta(const QPoint& deltaWidget)
{
  QSize videoWindowSize;
  QRect renderRect = updateRenderRect(videoWindowSize);

  const qreal sx = (qreal) videoWindowSize.width() / renderRect.width();
  const qreal sy = (qreal) videoWindowSize.height() / renderRect.height();

  return QPoint(deltaWidget.x() * sx, deltaWidget.y() * sy);
}

QPoint updateDelta(QPoint _delta, PreviewSceneWidget::Mode _mode)
{
  if((_mode == PreviewSceneWidget::ResizeBottomRight) || (_mode == PreviewSceneWidget::ResizeBottomLeft) || (_mode == PreviewSceneWidget::ResizeTopRight) || (_mode == PreviewSceneWidget::ResizeTopLeft))
  {
    return QPoint(_delta.x(), _delta.x());
  }

  return _delta;
}

void PreviewSceneWidget::mouseMoveEvent(QMouseEvent* e)
{
  if(activeRectIndex_ >= 0)
  {
    // dragging
    QPoint delta = e->pos() - lastMousePos_;
    delta = updateDelta(delta, mode_);
    delta = widgetDeltaToVideoDelta(delta);

    if(delta.x() != 0 && delta.y() != 0)
    {
      rects_[activeRectIndex_] = activeRect_;
      
      QRect& r = rects_[activeRectIndex_];
      if(mode_ == Move)
      {
        r.translate(delta);
      }
      else if(mode_ == PreviewSceneWidget::ResizeBottomRight)
      {
        r.setBottomRight(r.bottomRight() + delta);
        r = r.normalized();
      }
      else if(mode_ == PreviewSceneWidget::ResizeBottomLeft)
      {
        r.setBottomLeft(QPoint(r.bottomLeft().x() + delta.x(), r.bottomLeft().y() - delta.y() ));
        r = r.normalized();
      }
      else if(mode_ == PreviewSceneWidget::ResizeTopRight)
      {
        r.setTopRight(QPoint(r.topRight().x() + delta.x(), r.topRight().y() - delta.y()));
        r = r.normalized();
      }
      else if(mode_ == PreviewSceneWidget::ResizeTopLeft)
      {
        r.setTopLeft(QPoint(r.topLeft().x() + delta.x(), r.topLeft().y() + delta.y()));
        r = r.normalized();
      }
      else if(mode_ == PreviewSceneWidget::ResizeCenterRight)
      {
        QSize videoWindowSize;
        updateRenderRect(videoWindowSize);

        int newWidth = r.width() + delta.x();
        int w = videoWindowSize.width() - r.left();
        if((newWidth > (w - 20)) && (newWidth < (w + 20))) newWidth = w;

        r.setWidth(newWidth);
        r = r.normalized();
      }
      else if(mode_ == PreviewSceneWidget::ResizeCenterLeft)
      {
        int newLeft = r.left() + delta.x();
        if(abs(newLeft) < 20) newLeft = 0;
      
        r.setLeft(newLeft);
        r = r.normalized();
      }
      else if(mode_ == PreviewSceneWidget::ResizeTopCenter)
      {
        int newY = r.y() + delta.y();
        if(abs(newY) < 20) newY = 0;

        r.setY(newY);
        r = r.normalized();
      }
      else if(mode_ == PreviewSceneWidget::ResizeBottomCenter)
      {
        QSize videoWindowSize;
        updateRenderRect(videoWindowSize);

        int newHeight = r.height() + delta.y();
        int h = videoWindowSize.height() - r.top();
        if((newHeight > (h - 20)) && (newHeight < (h + 20))) newHeight = h;

        r.setHeight(newHeight);
        r = r.normalized();
      }

      setCursor(cursorForMode(mode_));
      update();
    }
    else
    {
      setCursor(cursorForMode(mode_));
    }

    return;
  }

  // to screen positions
  QVector<QRectF> rects = videoRectsToWidget();

  // ---- hover logic ----
  int idx = findRectAt(rects, e->pos());
  if(idx < 0)
  {
    unsetCursor();
    return;
  }

  const QRectF& r = rects[idx];
  Mode hoverMode = modeForPosition(r, e->pos());
  setCursor(cursorForMode(hoverMode));
}

void PreviewSceneWidget::mouseReleaseEvent(QMouseEvent*)
{
  if(activeRectIndex_ >= 0)
  {
    emit onCurrentSourceRectChange(rects_[activeRectIndex_]);
  }

  activeRectIndex_ = -1;
  mode_ = None;
}

void PreviewSceneWidget::leaveEvent(QEvent*)
{
  if(activeRectIndex_ < 0)
  {
    unsetCursor();
  }
}