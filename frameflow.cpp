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
#include "createscenedialog.h"
#include "newprojectdialog.h"
#include "renamedialog.h"
#include "renderermanager.h"

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

  auto modelScene = ui.sceneListWidget->model();
  connect(modelScene, &QAbstractItemModel::rowsInserted, this, [this]() { onUpdateNumScenes(); });
  connect(modelScene, &QAbstractItemModel::rowsRemoved, this, [this]() { onUpdateNumScenes(); });
  connect(modelScene, &QAbstractItemModel::modelReset, this, [this]() { onUpdateNumScenes(); });
  onUpdateNumScenes();

  auto modelSource = ui.sourceListWidget->model();
  connect(modelSource, &QAbstractItemModel::rowsInserted, this, [this]() { onUpdateNumSources(); });
  connect(modelSource, &QAbstractItemModel::rowsRemoved, this, [this]() { onUpdateNumSources(); });
  connect(modelSource, &QAbstractItemModel::modelReset, this, [this]() { onUpdateNumSources(); });
  onUpdateNumSources();

  // dummy transitions
  for(int i = 0; i < 5; i++)
  {
    QListWidgetItem* lwi = new QListWidgetItem(ui.transitionListWidget);
    lwi->setSizeHint(QSize(100, 35));
    TransitionWidget* sw = new TransitionWidget();
    ui.transitionListWidget->addItem(lwi);
    ui.transitionListWidget->setItemWidget(lwi, sw);
  }

  ui.transitionListWidget->item(0)->setSelected(true); // cut
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

  // Program | Preview
  connect(ui.buttonGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), this, [this] (QAbstractButton* button) {
    if(button == ui.directButton) {
      ui.stackedWidget->setCurrentIndex(0);
    }
    else if(button == ui.previewButton) {
      ui.stackedWidget->setCurrentIndex(1);
    }
  });

  ui.previewSceneWidget->setPreviewMode(EPreviewMode::PM_PROGRAM);
  ui.PGCWidget->setPreviewMode(EPreviewMode::PM_PROGRAM);
  ui.PREVIEWWidget->setPreviewMode(EPreviewMode::PM_PREVIEW);
}

FrameFlow::~FrameFlow()
{
  // unload renderers
  RendererManager& rm = RendererManager::instance();
  rm.unload();

  projectsWidget_->deleteLater();

  // mandatory. invalid signals received
  if(auto m = ui.sourceListWidget->model()) { disconnect(m, nullptr, this, nullptr);}
  if(auto m = ui.sceneListWidget->model()) { disconnect(m, nullptr, this, nullptr); }
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
    if(w)
    {
      bool selected = item->isSelected();
      w->setSelected(selected);
      if(selected)
      {
        ProjectManager& pm = ProjectManager::instance();
        QUuid id = w->id();
        pm.setCurrentScene(id);
      }
    }
  }

  updateSources();
}

void FrameFlow::onSourceSelectionChanged()
{
  for(int i = 0; i < ui.sourceListWidget->count(); ++i)
  {
    QListWidgetItem* item = ui.sourceListWidget->item(i);
    SourceWidget* w = static_cast<SourceWidget*>(ui.sourceListWidget->itemWidget(item));
    if(w) w->setSelected(item->isSelected());
  }
}

void FrameFlow::onTransitionSelectionChanged()
{
  for(int i = 0; i < ui.transitionListWidget->count(); ++i)
  {
    QListWidgetItem* item = ui.transitionListWidget->item(i);
    TransitionWidget* w = static_cast<TransitionWidget*>(ui.transitionListWidget->itemWidget(item));
    if(w) w->setSelected(item->isSelected());
  }
}

void FrameFlow::onEffectSelectionChanged()
{
  for(int i = 0; i < ui.effectListWidget->count(); ++i)
  {
    QListWidgetItem* item = ui.effectListWidget->item(i);
    EffectWidget* w = static_cast<EffectWidget*>(ui.effectListWidget->itemWidget(item));
    if(w) w->setSelected(item->isSelected());
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
  dlg.move(screenGeometry.center() - dlg.rect().center());

  if(dlg.exec() == QDialog::Accepted)
  {
    Source source = dlg.source();
    ProjectManager& pm = ProjectManager::instance();
    pm.addSource(source);
  }
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
  updateScenes();

  // notify to renderer
  RendererManager &rm = RendererManager::instance();
  rm.reloadProjec();

  ui.outputValueLabel->setText(BaseRenderer::getVideoFormatString(project->width, project->height, project->framerate));
  ui.previewSceneLabel->setText(QString("%1x%2  %3fps  8.500 kbps").arg(project->width).arg(project->height).arg(project->framerate));
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

void FrameFlow::onCreateScene()
{
  CreateSceneDialog dlg;

  // center
  QScreen* screen = QGuiApplication::screenAt(QCursor::pos());
  if(!screen) screen = QGuiApplication::primaryScreen();
  QRect screenGeometry = screen->availableGeometry();
  dlg.move(screenGeometry.center() - dlg.rect().center());
  if(dlg.exec() == QDialog::Accepted)
  {
    QString name = dlg.name();
    
    ProjectManager& pm = ProjectManager::instance();
    auto project = pm.currentProject();
    if(project)
    {
      Scene scene = {};
      scene.name = name;
      scene.orderIndex = project->scenes.size();
      pm.addScene(scene);
    }    
  }
}

void FrameFlow::onUpdateNumSources()
{
  ui.sourcesCountLabel->setText(QString("(%1)").arg(ui.sourceListWidget->count()));
  if(ui.sourceListWidget->count() > 0) ui.noSourceWidget->hide();
  else ui.noSourceWidget->show();
}

void FrameFlow::onUpdateNumScenes()
{
  ui.scenesCountLabel->setText(QString("(%1)").arg(ui.sceneListWidget->count()));
}

void FrameFlow::updateScenes()
{
  ProjectManager& pm = ProjectManager::instance();
  auto project = pm.currentProject();
  if(!project) return;

  // clear
  while(ui.sceneListWidget->count() > 0)
  {
    QListWidgetItem* it = ui.sceneListWidget->takeItem(0);
    delete it;
  }

  // populate
  for(int i = 0; i < project->scenes.size(); i++)
  {
    Scene scene = project->scenes[i];
    QListWidgetItem* lwi = new QListWidgetItem(ui.sceneListWidget);
    lwi->setSizeHint(QSize(100, 70));
    SceneWidget* sw = new SceneWidget(scene);
    connect(sw, &SceneWidget::onDeleteScene, this, [this, scene] () { deleteScene(scene); });
    connect(sw, &SceneWidget::onRenameScene, this, [this, scene]() { renameScene(scene); });
    ui.sceneListWidget->addItem(lwi);
    ui.sceneListWidget->setItemWidget(lwi, sw);
  }

  bool selected = false;
  QUuid sceneId = pm.currentSceneId();
  for(int i = 0; i < project->scenes.size() && !selected; i++)
  {
    if(project->scenes[i].id == sceneId)
    {
      ui.sceneListWidget->item(i)->setSelected(true);
      selected = true;
    }
  }
  if(!selected && ui.sceneListWidget->count() > 0)
  {
    ui.sceneListWidget->item(0)->setSelected(true);
  }
}

void FrameFlow::updateSources()
{
  ProjectManager& pm = ProjectManager::instance();
  auto project = pm.currentProject();
  if(!project) return;

  // clear
  while(ui.sourceListWidget->count() > 0)
  {
    QListWidgetItem* it = ui.sourceListWidget->takeItem(0);
    delete it;
  }

  QUuid sceneID = pm.currentSceneId();
  // populate
  for(int i = 0; i < project->scenes.size(); i++)
  {
    Scene scene = project->scenes[i];
    if(scene.id == sceneID)
    {
      for(int j = 0; j < scene.sources.size(); j++)
      {
        QListWidgetItem* lwi = new QListWidgetItem(ui.sourceListWidget);
        lwi->setSizeHint(QSize(0, 30));
        SourceWidget* sw = new SourceWidget(scene.sources[j]);
        ui.sourceListWidget->addItem(lwi);
        ui.sourceListWidget->setItemWidget(lwi, sw);
      }
    }
  }
}

void FrameFlow::deleteScene(const Scene& _scene)
{
  if(ui.sceneListWidget->count() == 1) return;

  QMessageBox::StandardButton reply = ConfirmationDialog::question(this, "Remove scene", "Are you sure you want to remove this scene?", QMessageBox::Yes, QMessageBox::No, QMessageBox::No);
  if(reply == QMessageBox::Yes)
  {
    ProjectManager& pm = ProjectManager::instance();
    pm.removeScene(_scene.id);
  }
}

void FrameFlow::renameScene(const Scene& _scene)
{
  RenameDialog dlg(_scene.name, this);
  dlg.setWindowModality(Qt::ApplicationModal);

  // center
  QScreen* screen = QGuiApplication::screenAt(QCursor::pos());
  if(!screen) screen = QGuiApplication::primaryScreen();
  QRect screenGeometry = screen->availableGeometry();
  dlg.move(screenGeometry.center() - dlg.rect().center());

  if(dlg.exec() == QDialog::Accepted)
  {
    ProjectManager& pm = ProjectManager::instance();
    auto newName = dlg.newName();
    pm.renameScene(_scene.id, newName);
  }
}
