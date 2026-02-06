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

  // sample rect to resize and move
  rects_.append(QRect(50, 50, 120, 80));
  update();
}

PreviewSceneWidget::~PreviewSceneWidget()
{

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

QRect PreviewSceneWidget::updateRenderRect()
{
  if(width() <= 0 || height() <= 0) return QRect();

  ProjectManager &pm = ProjectManager::instance();
  auto currentProject = pm.currentProject();
  if(!currentProject) return QRect();
  QSize videoWindowSize = { currentProject->width, currentProject->height };

  const double sx = double(width()) / videoWindowSize.width();
  const double sy = double(height()) / videoWindowSize.height();
  const double scale = (std::min)(sx, sy);

  const QSize fitted(int(videoWindowSize.width() * scale), int(videoWindowSize.height() * scale));
  const QPoint topLeft((width() - fitted.width()) / 2, (height() - fitted.height()) / 2);
  return QRect(topLeft, fitted);
}

int findRectAt(const QVector<QRect>& rects, const QPoint& pos)
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
    case PreviewSceneWidget::Move:   return Qt::SizeAllCursor;
    case PreviewSceneWidget::Resize: return Qt::SizeFDiagCursor;
    default:                         return Qt::ArrowCursor;
  }
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

  p.drawText(
    QRect(center.x() - 100, center.y() + 60, 200, 30),
    Qt::AlignCenter,
    "Preview: Main Scene"
  );

  // render
  QRect renderRect = PreviewSceneWidget::updateRenderRect();
  if(!ARGBImage_.isNull())
  {
    p.drawImage(renderRect, ARGBImage_);
  }

  // rects
  QPen rectPen(Qt::blue, 2);
  p.setPen(rectPen);
  for(const QRect& r : rects_)
  {
    p.drawRect(r);
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

void PreviewSceneWidget::mousePressEvent(QMouseEvent* e)
{
  activeRectIndex_ = findRectAt(rects_, e->pos());
  lastMousePos_ = e->pos();
  mode_ = None;

  if(activeRectIndex_ >= 0)
  {
    QRect& r = rects_[activeRectIndex_];
    QRect resizeHandle(r.bottomRight() - QPoint(10, 10), r.bottomRight());
    mode_ = resizeHandle.contains(e->pos()) ? Resize : Move;
    setCursor(cursorForMode(mode_));
  }
}


void PreviewSceneWidget::mouseMoveEvent(QMouseEvent* e)
{
  if(activeRectIndex_ >= 0)
  {
    // dragging
    QPoint delta = e->pos() - lastMousePos_;
    QRect& r = rects_[activeRectIndex_];

    if(mode_ == Move)
    {
      r.translate(delta);
    }
    else if(mode_ == Resize)
    {
      r.setBottomRight(r.bottomRight() + delta);
      r = r.normalized();
    }

    lastMousePos_ = e->pos();
    setCursor(cursorForMode(mode_));
    update();
    return;
  }

  // ---- hover logic ----
  int idx = findRectAt(rects_, e->pos());
  if(idx < 0) {
    unsetCursor();
    return;
  }

  const QRect& r = rects_[idx];

  QRect resizeHandle(r.bottomRight() - QPoint(10, 10), r.bottomRight());

  Mode hoverMode = resizeHandle.contains(e->pos()) ? Resize : Move;
  setCursor(cursorForMode(hoverMode));
}

void PreviewSceneWidget::mouseReleaseEvent(QMouseEvent*)
{
  activeRectIndex_ = -1;
  mode_ = None;
}

void PreviewSceneWidget::leaveEvent(QEvent*)
{
  if(activeRectIndex_ < 0){
    unsetCursor();
  }
}