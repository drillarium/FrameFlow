#include "frameflow.h"
#include <QSettings>
#include <QTimer>
#include "myutils.h"
#include "widgetbutton.h"
#include "projectswidget.h"
#include "settingsdialog.h"
#include "alertsdialog.h"
#include "confirmationdialog.h"

FrameFlow::FrameFlow(QWidget *_parent)
:QMainWindow(_parent)
{
  ui.setupUi(this);
  
  readSettings();
  onSetWindowTitleVisible();

  // system state
  QTimer* timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &FrameFlow::updateSystemStats);
  timer->start(1000);
  
  // project button
  projectsWidget_ = new ProjectsWidget(this);
  connect(ui.projectButtonWidget, &WidgetButton::clicked, this, [=]() {
    // projects widget
    if(projectsWidget_->isVisible())
    {
      projectsWidget_->hide();
    }
    else
    {
      QPoint pos = ui.projectButtonWidget->mapToGlobal(QPoint(0, ui.projectButtonWidget->height()));
      projectsWidget_->move(pos);
      projectsWidget_->show();
      projectsWidget_->raise();
      projectsWidget_->activateWindow();
    }
  });
}

FrameFlow::~FrameFlow()
{
  projectsWidget_->deleteLater();
}

void FrameFlow::readSettings()
{
  QSettings settings("AVIO", "FrameFlow");

  // Geometry & UI state
  if(settings.contains("geometry"))
  {
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
  }
  else
  {
    // First run: start maximized
    showMaximized();
    return;
  }

  // Restore window mode
  const QString mode = settings.value("mode", "normal").toString();

  if(mode == "maximized")
  {
    showMaximized();
  }
  else if(mode == "fullscreen")
  {
    showFullScreen();
  }
  else
  {
    showNormal();
  }
}

void FrameFlow::writeSettings()
{
  QSettings settings("MyCompany", "MyApp");

  settings.setValue("geometry", saveGeometry());
  settings.setValue("windowState", saveState());

  // Save window mode explicitly
  if(isFullScreen())
  {
    settings.setValue("mode", "fullscreen");
  }
  else if(isMaximized())
  {
    settings.setValue("mode", "maximized");
  }
  else
  {
    settings.setValue("mode", "normal");
  }
}

void FrameFlow::closeEvent(QCloseEvent* event)
{
  QMessageBox::StandardButton reply = ConfirmationDialog::question(this, "Exit application", "Are you sure you want to exit?", QMessageBox::Yes, QMessageBox::No, QMessageBox::No);
  if(reply == QMessageBox::Yes)
  {
    writeSettings();
    event->accept();
  }
  else
  {
    event->ignore();
  }
}

void FrameFlow::updateSystemStats()
{
  double cpu = cpuUsage();
  MemoryInfo mem = memoryUsage();
  double gpu = 0;

  ui.cpuValueLabel->setText(QString("%1%").arg(cpu, 0, 'f', 1));
  ui.gpuValueLabel->setText(QString("%1%").arg(gpu, 0, 'f', 1));
  ui.memValueLabel->setText(QString("%1% / %2 GB").arg(mem.usedPercent, 0, 'f', 1).arg(mem.usedGB, 0, 'f', 1));
}

void FrameFlow::onAlerts()
{
  AlertsDialog dlg(this);
  dlg.setWindowModality(Qt::ApplicationModal);

  QWidget* parent = this;
  QRect parentRect = parent->geometry();
  QPoint topLeft = parent->mapToGlobal(QPoint(0, 0));
  int x = topLeft.x() + parentRect.width() - dlg.width();
  int y = topLeft.y();
  dlg.setGeometry(x, y, dlg.width(), parentRect.height());
  dlg.exec();
}

void FrameFlow::onSettings()
{
  SettingsDialog dlg(this);
  dlg.setWindowModality(Qt::ApplicationModal);

  QWidget* parent = this;
  QRect parentRect = parent->geometry();
  QPoint topLeft = parent->mapToGlobal(QPoint(0, 0));
  int x = topLeft.x() + parentRect.width() - dlg.width();
  int y = topLeft.y();
  dlg.setGeometry(x, y, dlg.width(), parentRect.height());
  dlg.exec();
}

void FrameFlow::onSetWindowTitleVisible()
{
  Qt::WindowFlags flags = windowFlags();

  windowTitleVisible_ = !windowTitleVisible_;
  if(windowTitleVisible_)
  {
    flags &= ~Qt::FramelessWindowHint;
  }
  else
  {
    flags |= Qt::FramelessWindowHint;
  }

  setWindowFlags(flags);

  show();
}
