#include "frameflow.h"
#include <QSettings>
#include <QTimer>
#include "myutils.h"
#include "widgetbutton.h"
#include "projectswidget.h"
#include "settingsdialog.h"
#include "alertsdialog.h"
#include "confirmationdialog.h"
#include "scenewidget.h"
#include "sourcewidget.h"
#include "transitionwidget.h"
#include "effectwidget.h"
#include "selectsourcedialog.h"
#include "project_manager.h"
#include <QStandardPaths>
#include <QDir>

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

  // db path
  QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QDir dir(dataDir);
  QString dbPath = dir.filePath("frameflow.db");

  // project manager
  ProjectManager& pm = ProjectManager::instance();
  connect(&pm, &ProjectManager::currentProjectChanged, this, &FrameFlow::onCurrentProjectChange);
  connect(&pm, &ProjectManager::projectListChanged, this, &FrameFlow::onProjectListChanged);
  connect(&pm, &ProjectManager::dirtyChanged, this, &FrameFlow::onProjectDirtyChange);

  // open DB and create a first project case no project available
  pm.openDatabase(dbPath);
  auto projects = pm.listProjects();
  if(projects.size() == 0)
  {
    pm.createProject("My Project");
  }
  projects = pm.listProjects();
  if(projects.size() > 0)
  {
    pm.loadProject(projects[0].id);
  }
  
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

  // dummy scenes
  for(int i = 0; i < 5; i++)
  {
    QListWidgetItem* lwi = new QListWidgetItem(ui.sceneListWidget);
    lwi->setSizeHint(QSize(100, 100));
    SceneWidget* sw = new SceneWidget();
    ui.sceneListWidget->addItem(lwi);
    ui.sceneListWidget->setItemWidget(lwi, sw);
  }
  ui.sceneListWidget->setCurrentRow(0); // first

  // dummy sources
  for(int i = 0; i < 5; i++)
  {
    QListWidgetItem* lwi = new QListWidgetItem(ui.sourceListWidget);
    lwi->setSizeHint(QSize(0, 35));
    SourceWidget* sw = new SourceWidget();
    ui.sourceListWidget->addItem(lwi);
    ui.sourceListWidget->setItemWidget(lwi, sw);
  }

  // dummy transitions
  for(int i = 0; i < 5; i++)
  {
    QListWidgetItem* lwi = new QListWidgetItem(ui.transitionListWidget);
    lwi->setSizeHint(QSize(100, 35));
    TransitionWidget* sw = new TransitionWidget();
    ui.transitionListWidget->addItem(lwi);
    ui.transitionListWidget->setItemWidget(lwi, sw);
  }
  ui.transitionListWidget->setCurrentRow(0); // cut
  ui.transitionsStackedWidget->setCurrentIndex(1);
  onExpandTransitions();

  // dummy effects
  for(int i = 0; i < 2; i++)
  {
    QListWidgetItem* lwi = new QListWidgetItem(ui.effectListWidget);
    lwi->setSizeHint(QSize(50, 50));
    EffectWidget* sw = new EffectWidget();
    ui.effectListWidget->addItem(lwi);
    ui.effectListWidget->setItemWidget(lwi, sw);
  }
  ui.effectsStackedWidget->setCurrentIndex(1);
  onExpandEffects();

  connect(ui.buttonGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), this, [this] (QAbstractButton* button) {
    if(button == ui.directButton) {
      ui.stackedWidget->setCurrentIndex(0);
    }
    else if(button == ui.previewButton) {
      ui.stackedWidget->setCurrentIndex(1);
    }
  });

  ui.PGCWidget->setPreviewMode(EPreviewMode::PM_PROGRAM);
  ui.PREVIEWWidget->setPreviewMode(EPreviewMode::PM_PREVIEW);
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
#ifndef _DEBUG
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
#else
  writeSettings();
  event->accept();
#endif // _DEBUG
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

  if(!windowTitleVisible_) showFullScreen();
  else show();
}

void FrameFlow::onSceneSelectionChanged()
{
  for(int i = 0; i < ui.sceneListWidget->count(); ++i)
  {
    QListWidgetItem* item = ui.sceneListWidget->item(i);
    SceneWidget* w = static_cast<SceneWidget*>(ui.sceneListWidget->itemWidget(item));
    w->setSelected(item->isSelected());
  }
}

void FrameFlow::onSourceSelectionChanged()
{
  for(int i = 0; i < ui.sourceListWidget->count(); ++i)
  {
    QListWidgetItem* item = ui.sourceListWidget->item(i);
    SourceWidget* w = static_cast<SourceWidget*>(ui.sourceListWidget->itemWidget(item));
    w->setSelected(item->isSelected());
  }
}

void FrameFlow::onTransitionSelectionChanged()
{
  for(int i = 0; i < ui.transitionListWidget->count(); ++i)
  {
    QListWidgetItem* item = ui.transitionListWidget->item(i);
    TransitionWidget* w = static_cast<TransitionWidget*>(ui.transitionListWidget->itemWidget(item));
    w->setSelected(item->isSelected());
  }
}

void FrameFlow::onEffectSelectionChanged()
{
  for(int i = 0; i < ui.effectListWidget->count(); ++i)
  {
    QListWidgetItem* item = ui.effectListWidget->item(i);
    EffectWidget* w = static_cast<EffectWidget*>(ui.effectListWidget->itemWidget(item));
    w->setSelected(item->isSelected());
  }
}

void FrameFlow::onExpandTransitions()
{
  int index = ui.transitionsStackedWidget->currentIndex();
  if(index == 0)
  {
    ui.expandTransitionButton->setText("Collapse");
    ui.transitionsStackedWidget->setCurrentIndex(1);
    ui.transitionsStackedWidget->setFixedHeight(100);
  }
  else
  {
    ui.expandTransitionButton->setText("Expand");
    ui.transitionsStackedWidget->setCurrentIndex(0);
    ui.transitionsStackedWidget->setFixedHeight(50);
  }
}

void FrameFlow::onExpandEffects()
{
  int index = ui.effectsStackedWidget->currentIndex();
  if(index == 0)
  {
    ui.expandEffectButton->setText("Collapse");
    ui.effectsStackedWidget->setCurrentIndex(1);
    ui.effectsStackedWidget->setFixedHeight(100);
  }
  else
  {
    ui.expandEffectButton->setText("Expand");
    ui.effectsStackedWidget->setCurrentIndex(0);
    ui.effectsStackedWidget->setFixedHeight(50);
  }
}

void FrameFlow::keyPressEvent(QKeyEvent* event)
{
  if(event->key() == Qt::Key_F11)
  {
    onSetWindowTitleVisible();
  }
  else
  {
    QMainWindow::keyPressEvent(event);
  }
}

void FrameFlow::onAddSource()
{
  SelectSourceDialog dlg(this);
  dlg.setWindowModality(Qt::ApplicationModal);

  // center
  QScreen* screen = QGuiApplication::screenAt(QCursor::pos());
  if(!screen) screen = QGuiApplication::primaryScreen();
  QRect screenGeometry = screen->availableGeometry();
  // dlg.adjustSize();
  dlg.move(screenGeometry.center() - dlg.rect().center());

  dlg.exec();
}

void FrameFlow::onCurrentProjectChange()
{
  ProjectManager& pm = ProjectManager::instance();
  auto project = pm.currentProject();
  if(!project) return;

  ui.currentProjectNameLabel->setText(project->name);

  if(projectsWidget_)
  {
    projectsWidget_->setCurrentProject(project->id);
  }
}

void FrameFlow::onProjectListChanged()
{
  ProjectManager& pm = ProjectManager::instance();
  auto projects = pm.listProjects();

  if(projectsWidget_)
  {
    projectsWidget_->updateProjectList();
  }
}

void FrameFlow::onProjectDirtyChange()
{

}
