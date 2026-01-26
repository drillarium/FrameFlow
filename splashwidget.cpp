#include "splashwidget.h"
#include <QApplication>
#include <QScreen>

SplashWidget::SplashWidget(QWidget* parent)
:QWidget(parent)
{
  ui.setupUi(this);

  setWindowFlags(Qt::FramelessWindowHint | Qt::SplashScreen | Qt::WindowStaysOnTopHint);
  setAttribute(Qt::WA_TranslucentBackground);

  // Center on screen
  QRect screen = QGuiApplication::primaryScreen()->geometry();
  move(screen.center().x() - width() / 2, screen.center().y() - height() / 2);
}

void SplashWidget::setMessage(const QString& text, int step, int totalSteps)
{
  ui.statusLabel->setText(text);
  ui.progressBar->setValue((int) ((step * 100.) / totalSteps));

  qApp->processEvents();
}
