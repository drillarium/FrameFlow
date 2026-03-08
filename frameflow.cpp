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
#include "editsourcedialog.h"
#include "helpdialog.h"
#include "transition_model.h"
#include "MLProtect_MFormats SDK.(subscription valid until 25-May-2025 - NRD Multimedia, S.L.).h"
#include <QJsonDocument>
#include "notification_manager.h"
#include "vumetercontrol.h"
#include <QToolButton>
#include "effectsdialog.h"

// MFormatProtectionInitializer
class MFormatProtectionInitializer
{
public:
  MFormatProtectionInitializer()
  {
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    hr = MFormatsSDKLic::IntializeProtection();
  }
  ~MFormatProtectionInitializer()
  {
    MFormatsSDKLic::CloseProtection();
    CoUninitialize();
  }

} initializer_;

// FrameFlow
FrameFlow::FrameFlow(QWidget *_parent)
:QMainWindow(_parent)
{
  ui.setupUi(this);

  LicenseManager &lm = LicenseManager::instance();
  connect(&lm, &LicenseManager::statusChanged, this, &FrameFlow::onLicenseChanged);
  showLicenseStatus(lm.status());
  
  readSettings();

  // system state
  QTimer* timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &FrameFlow::updateSystemStats);
  timer->start(1000);

  timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &FrameFlow::updateProjectInfo);
  timer->start(100);

  // db path
  QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QDir dir(dataDir);
  QString dbPath = dir.filePath("frameflow.db");

  // project manager
  ProjectManager& pm = ProjectManager::instance();
  connect(&pm, &ProjectManager::currentProjectChanged, this, &FrameFlow::onCurrentProjectChange);
  connect(&pm, &ProjectManager::projectListChanged, this, &FrameFlow::onProjectListChanged);

  // open DB and create a first project case no project available
  pm.loadEncodingSettings();
  pm.openDatabase(dbPath);
  auto projects = pm.listProjects();
  if(projects.size() == 0)
  {
    pm.createProject("My Project");
  }
  projects = pm.listProjects();
  if(projects.size() > 0)
  {
    if(nextProjectUID_.isNull())
    {
      pm.loadProject(projects[0].id);
    }
    else
    {
      pm.loadProject(nextProjectUID_);
    }
  }

  if(!nextTransitionUID_.isNull())
  {
    pm.setCurrentTransition(nextTransitionUID_);
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

  // CUT | FADE | SLIDE
  auto transitions = pm.listTransitions();
  int selectedTransitionIndex = -1;
  auto transition = pm.currentTransition();
  for(int i = 0; i < transitions.size(); i++)
  {
    QListWidgetItem* lwi = new QListWidgetItem(ui.transitionListWidget);
    lwi->setSizeHint(QSize(100, 35));
    Transition t = transitions[i];
    TransitionWidget* sw = new TransitionWidget(t);
    ui.transitionListWidget->addItem(lwi);
    ui.transitionListWidget->setItemWidget(lwi, sw);
    if(transition && transition->id == t.id)
    {
      selectedTransitionIndex = i;
    }
  }
  if( (selectedTransitionIndex < 0) && (transitions.size() > 0) )
  {
    selectedTransitionIndex = 0;
    pm.setCurrentTransition(transitions[0].id);
  }
  if(selectedTransitionIndex >= 0)
  {
    ui.transitionListWidget->item(selectedTransitionIndex)->setSelected(true);
  }
  ui.transitionsStackedWidget->setCurrentIndex(1);
  onExpandTransitions();

  // dummy effects
  for(int i = 0; i < 2; i++)
  {
    QListWidgetItem* lwi = new QListWidgetItem(ui.effectListWidget);
    lwi->setSizeHint(QSize(0, 30));
    EffectWidget* sw = new EffectWidget();
    ui.effectListWidget->addItem(lwi);
    ui.effectListWidget->setItemWidget(lwi, sw);
  }

  // dummy actions
  for(int i = 0; i < 3; i++)
  {
    QToolButton*action = new QToolButton();
    action->setFixedSize(50, 50);
    action->setCursor(Qt::PointingHandCursor);
    action->setIconSize(QSize(16, 16));
    action->setIcon(QIcon(":/FrameFlow/camera.svg"));
    action->setStyleSheet("QToolButton { background: #303541; border: 1px solid black; border-radius: 10px; padding: 6px 6px; color: white; font-size: 8px;}");
    action->setText("NDI");
    action->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    QListWidgetItem* lwi = new QListWidgetItem(ui.actionsListWidget);
    lwi->setSizeHint(QSize(65, 55));
    ui.actionsListWidget->addItem(lwi);
    ui.actionsListWidget->setItemWidget(lwi, action);
  }

  // Program | Preview
  connect(ui.buttonGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), this, [this] (QAbstractButton* button) {
    if(button == ui.directButton) {
      ProjectManager &pm = ProjectManager::instance();
      pm.setWorkingMode(ProjectManager::CONT);
      ui.stackedWidget->setCurrentIndex(0);

      // select scene from Studio
      QUuid studio = pm.currentStudioSceneId();
      selectScene(studio);
    }
    else if(button == ui.previewButton) {
      ProjectManager& pm = ProjectManager::instance();
      pm.setWorkingMode(ProjectManager::STUDIO);
      ui.stackedWidget->setCurrentIndex(1);
    }
  });

  ui.previewSceneWidget->setPreviewMode(EPreviewMode::PM_PROGRAM);
  ui.PGCWidget->setPreviewMode(EPreviewMode::PM_PROGRAM);
  ui.PREVIEWWidget->setPreviewMode(EPreviewMode::PM_PREVIEW);

  connect(ui.previewSceneWidget, &PreviewSceneWidget::onCurrentSourceRectChange, this, &FrameFlow::onCurrentSourceRectChange);
  connect(ui.PREVIEWWidget, &PreviewSceneWidget::onCurrentSourceRectChange, this, &FrameFlow::onCurrentSourceRectChange);

  // live status signals
  RendererManager& rm = RendererManager::instance();
  connect(&rm, &RendererManager::onStreamingStateChange, this, &FrameFlow::onStreamingStateChange);
  onStreamingStateChange(EStreamingState::SS_NONE);

  connect(&rm, &RendererManager::onRecordingStateChange, this, &FrameFlow::onRecordingStateChange);
  onRecordingStateChange(ERecordingState::RS_NONE);

  // notifications
  NotificationManager& nm = NotificationManager::instance();
  connect(&nm, &NotificationManager::onNotificationAdded, this, &FrameFlow::updateNotifications);
  connect(&nm, &NotificationManager::onNotificationUpdated, this, &FrameFlow::updateNotifications);
  connect(&nm, &NotificationManager::onNotificationRemoved, this, &FrameFlow::updateNotifications);

  initExternalAudio();

#ifndef _DEBUG
  ui.effectsWidget->hide();
  ui.timelineWidget->hide();
#endif // _DEBUG

}

FrameFlow::~FrameFlow()
{
  // unload renderers
  RendererManager& rm = RendererManager::instance();
  rm.unload();
  rm.deinit();

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
    showNormal();
  }
  else
  {
    showMaximized();
  }

  // Restore window mode
  const QString mode = settings.value("mode", "normal").toString();

  if(mode == "maximized")
  {
    showMaximized();
  }
  else if(mode == "fullscreen")
  {
    showMaximized();
    onSetWindowTitleVisible();
  }

  ProjectManager &pm = ProjectManager::instance();
  if(settings.contains("current_project"))
  {
    nextProjectUID_ = QUuid::fromString(settings.value("current_project").toString());
  }
  if(settings.contains("current_scene"))
  {
    QUuid scene = QUuid::fromString(settings.value("current_scene").toString());
    pm.setCurrentScene(scene);
  }
  if(settings.contains("current_transition"))
  {
    nextTransitionUID_ = QUuid::fromString(settings.value("current_transition").toString());
  }
}

void FrameFlow::writeSettings()
{
  QSettings settings("AVIO", "FrameFlow");

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

  ProjectManager &pm = ProjectManager::instance();
  auto project = pm.currentProject();
  if(project)
  {
    settings.setValue("current_project", project->id.toString());
    settings.setValue("current_scene", pm.currentSceneId().toString());
  }
  auto transition = pm.currentTransition();
  if(transition)
  {
    settings.setValue("current_transition", transition->id.toString());
  }
}

void FrameFlow::closeEvent(QCloseEvent* event)
{
#ifndef _DEBUG
  QMessageBox::StandardButton reply = ConfirmationDialog::question(this, "Exit application", "Are you sure you want to exit?", QMessageBox::Yes, QMessageBox::No, QMessageBox::Yes);
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

void FrameFlow::updateProjectInfo()
{
  // update streaming state
  if(streamingTimer_.isValid())
  {
    qint64 elapsed = streamingTimer_.elapsed();
    qint64 totalSeconds = elapsed / 1000;
    qint64 hours = totalSeconds / 3600;
    qint64 minutes = (totalSeconds % 3600) / 60;
    qint64 seconds = totalSeconds % 60;
    QString timeString = QString("%1:%2:%3").arg(hours, 2, 10, QChar('0')).arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
    ui.streamTimeLabel->setText(timeString);
    ui.streamingTimeLabel->setText(timeString);
  }
  else
  {
    ui.streamTimeLabel->setText("00:00:00");
    ui.streamingTimeLabel->setText("00:00:00");
  }

  // update recording state
  if(recordingTimer_.isValid())
  {
    qint64 elapsed = recordingTimer_.elapsed();
    qint64 totalSeconds = elapsed / 1000;
    qint64 hours = totalSeconds / 3600;
    qint64 minutes = (totalSeconds % 3600) / 60;
    qint64 seconds = totalSeconds % 60;
    QString timeString = QString("● %1:%2:%3").arg(hours, 2, 10, QChar('0')).arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
    ui.recordingLabel->setText(timeString);
  }
  else
  {
    ui.recordingLabel->setText("● 00:00:00");
  }

  // update vumeters
  RendererManager &rm = RendererManager::instance();
  for(int i = 0; i < vumeters_.size(); i++)
  {
    M_AUDIO_LOUDNESS al;
    if(rm.vumeterValue(i, al))
    {
      vumeters_[i]->setAudioLoudness(al);
    }
  }
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
  QUuid selectedSource;
  for(int i = 0; i < ui.sourceListWidget->count(); ++i)
  {
    QListWidgetItem* item = ui.sourceListWidget->item(i);
    SourceWidget* w = static_cast<SourceWidget*>(ui.sourceListWidget->itemWidget(item));
    if(w)
    {
      bool selected = item->isSelected();
      w->setSelected(selected);
      if(selected)
      {
        selectedSource = w->id();
      }
    }
  }

  ProjectManager& pm = ProjectManager::instance();
  pm.setCurrentSource(selectedSource);

  ui.previewSceneWidget->updateSelectedSource();
  ui.PREVIEWWidget->updateSelectedSource();
}

void FrameFlow::onTransitionSelectionChanged()
{
  for(int i = 0; i < ui.transitionListWidget->count(); ++i)
  {
    QListWidgetItem* item = ui.transitionListWidget->item(i);
    TransitionWidget* w = static_cast<TransitionWidget*>(ui.transitionListWidget->itemWidget(item));
    if(w)
    {
      w->setSelected(item->isSelected());
    }
  }

  onChangeTransitionSelected();
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
    ui.transitionsStackedWidget->setFixedHeight(30);
  }
}

void FrameFlow::onExpandEffects()
{
  EffectsDialog dlg(this);
  dlg.setWindowModality(Qt::ApplicationModal);

  // center
  QScreen* screen = QGuiApplication::screenAt(QCursor::pos());
  if(!screen) screen = QGuiApplication::primaryScreen();
  QRect screenGeometry = screen->availableGeometry();
  dlg.move(screenGeometry.center() - dlg.rect().center());

  dlg.exec();
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

  pm.resetDirty();

  ui.outputValueLabel->setText(BaseRenderer::getVideoFormatString(project->width, project->height, project->framerate));
  ui.previewSceneLabel->setText(QString("%1x%2  %3fps  0 kbps").arg(project->width).arg(project->height).arg(project->framerate));
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
  ui.sceneListWidget->blockSignals(true);
  while(ui.sceneListWidget->count() > 0)
  {
    QListWidgetItem* it = ui.sceneListWidget->takeItem(0);
    delete it;
  }
  ui.sceneListWidget->blockSignals(false);

  // populate
  for(int i = 0; i < project->scenes.size(); i++)
  {
    Scene scene = project->scenes[i];
    QListWidgetItem* lwi = new QListWidgetItem(ui.sceneListWidget);
    lwi->setSizeHint(QSize(100, 70));
    SceneWidget* sw = new SceneWidget(scene);
    connect(sw, &SceneWidget::onDeleteScene, this, [this, scene] () { deleteScene(scene); });
    connect(sw, &SceneWidget::onRenameScene, this, [this, scene]() { renameScene(scene); });
    connect(sw, &SceneWidget::onUpScene, this, [this, scene]() { moveScene(scene, true); });
    connect(sw, &SceneWidget::onDownScene, this, [this, scene]() { moveScene(scene, false); });
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
  ui.sourceListWidget->blockSignals(true);
  while(ui.sourceListWidget->count() > 0)
  {
    QListWidgetItem* it = ui.sourceListWidget->takeItem(0);
    delete it;
  }
  ui.sourceListWidget->blockSignals(false);

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
        Source source = scene.sources[j];
        SourceWidget* sw = new SourceWidget(source);
        connect(sw, &SourceWidget::onDeleteSource, this, [this, source]() { deleteSource(source); });
        connect(sw, &SourceWidget::onRenameSource, this, [this, source]() { renameSource(source); });
        connect(sw, &SourceWidget::onUpSource, this, [this, source]() { moveSource(source, true); });
        connect(sw, &SourceWidget::onDownSource, this, [this, source]() { moveSource(source, false); });
        ui.sourceListWidget->addItem(lwi);
        ui.sourceListWidget->setItemWidget(lwi, sw);
      }

      QUuid sourceId = pm.currentSourceId();
      bool selected = false;
      for(int j = 0; j < scene.sources.size() && !selected; j++)
      {
        selected = (scene.sources[j].id == sourceId);
        if(selected)
        {
          ui.sourceListWidget->item(j)->setSelected(true);
          break;
        }
      }
      
      if(!selected)
      {
        QUuid selectedUid;
        ProjectManager& pm = ProjectManager::instance();
        pm.setCurrentSource(selectedUid);

        ui.previewSceneWidget->updateSelectedSource();
        ui.PREVIEWWidget->updateSelectedSource();
      }

      break;
    }
  }
}

void FrameFlow::deleteScene(const Scene& _scene)
{
  if(ui.sceneListWidget->count() == 1) return;

  QMessageBox::StandardButton reply = ConfirmationDialog::question(this, "Remove scene", "Are you sure you want to remove this scene?", QMessageBox::Yes, QMessageBox::No, QMessageBox::Yes);
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

void FrameFlow::deleteSource(const Source& _source)
{ 
  QMessageBox::StandardButton reply = ConfirmationDialog::question(this, "Remove source", "Are you sure you want to remove this source?", QMessageBox::Yes, QMessageBox::No, QMessageBox::No);
  if(reply == QMessageBox::Yes)
  {
    ProjectManager& pm = ProjectManager::instance();
    pm.removeSource(_source.id);
  }
}

void FrameFlow::renameSource(const Source& _source)
{
  EditSourceDialog dlg(_source, this);
  dlg.setWindowModality(Qt::ApplicationModal);

  // center
  QScreen* screen = QGuiApplication::screenAt(QCursor::pos());
  if(!screen) screen = QGuiApplication::primaryScreen();
  QRect screenGeometry = screen->availableGeometry();
  dlg.move(screenGeometry.center() - dlg.rect().center());

  if(dlg.exec() == QDialog::Accepted)
  {
    ProjectManager& pm = ProjectManager::instance();
    auto newSource = dlg.newSource();
    pm.updateSource(newSource);
  }
}

void FrameFlow::onCurrentSourceRectChange(const QRect& _r)
{
  ProjectManager& pm = ProjectManager::instance();
  pm.updateSourceRect(_r);
}

void FrameFlow::onTake()
{
  ProjectManager& pm = ProjectManager::instance();
  QUuid nextScene = pm.currentSceneId();
  QUuid studioScene = pm.currentStudioSceneId();
  
  // switch. Force selection of studioScene
  selectScene(studioScene);


  // set scene
  pm.setCurrentStudioSceneId(nextScene);
}

void FrameFlow::selectScene(QUuid scene)
{
  for(int i = 0; i < ui.sceneListWidget->count(); ++i)
  {
    QListWidgetItem* item = ui.sceneListWidget->item(i);
    SceneWidget* w = static_cast<SceneWidget*>(ui.sceneListWidget->itemWidget(item));
    if(w)
    {
      if(w->id() == scene)
      {
        item->setSelected(true);
      }
    }
  }
}

void FrameFlow::moveSource(const Source& _source, bool up)
{
  ProjectManager& pm = ProjectManager::instance();
  pm.moveSource(_source.id, up);
}

void FrameFlow::moveScene(const Scene& _scene, bool up)
{
  ProjectManager& pm = ProjectManager::instance();
  pm.moveScene(_scene.id, up);
}

void FrameFlow::onLicenseChanged(LicenseManager::Status newStatus)
{
  showLicenseStatus(newStatus);
}

void FrameFlow::showLicenseStatus(LicenseManager::Status status)
{

}

void FrameFlow::onHelp()
{
  HelpDialog dlg(this);
  dlg.setWindowModality(Qt::ApplicationModal);

  // center
  QScreen* screen = QGuiApplication::screenAt(QCursor::pos());
  if(!screen) screen = QGuiApplication::primaryScreen();
  QRect screenGeometry = screen->availableGeometry();
  dlg.move(screenGeometry.center() - dlg.rect().center());

  dlg.exec();
}

void FrameFlow::onToggleFullScreen()
{
  fullScreen_ = !fullScreen_;

  if(fullScreen_)
  {
    ui.inputWidget->hide();
    ui.outputWidget->hide();
    ui.timelineWidget->hide();
    ui.previewFooterWidget->hide();
    ui.fullScreenButton->setIcon(QIcon(":/FrameFlow/exit-full-screen.svg"));
  }
  else
  {
    ui.inputWidget->show();
    ui.outputWidget->show();
    if(!ui.timelineButton->isChecked()) ui.timelineWidget->show();
    ui.previewFooterWidget->show();
    ui.fullScreenButton->setIcon(QIcon(":/FrameFlow/full-screen.svg"));
  }
}

void FrameFlow::onChangeTransitionDuration()
{
  for(int i = 0; i < ui.transitionListWidget->count(); ++i)
  {
    QListWidgetItem* item = ui.transitionListWidget->item(i);
    if(item->isSelected())
    {
      int ms = ui.transitionSlider->value();
      TransitionWidget* w = static_cast<TransitionWidget*>(ui.transitionListWidget->itemWidget(item));
      if(w)
      {
        Transition t = w->transition();
        t.msDuration = ms;
        ui.durationLabel->setText(t.type == TransitionType::TT_CUT ? "Instant" : QString("%1 ms %2").arg(t.msDuration).arg(defaultNameForTransition(t.type)));
        ui.currentDurationLabel->setText(QString("%1ms").arg(t.msDuration));
        w->setTransition(t);
      }
      break;
    }
  }
}

void FrameFlow::onChangeTransitionSelected()
{
  for(int i = 0; i < ui.transitionListWidget->count(); ++i)
  {
    QListWidgetItem* item = ui.transitionListWidget->item(i);
    if(item->isSelected())
    {
      TransitionWidget* w = static_cast<TransitionWidget*>(ui.transitionListWidget->itemWidget(item));
      if(w)
      {
        Transition t = w->transition();
        int ms = t.msDuration;
        ui.transitionSlider->setValue(ms);
        ui.durationLabel->setText(t.type == TransitionType::TT_CUT ? "Instant" : QString("%1 ms %2").arg(t.msDuration).arg(defaultNameForTransition(t.type)));
        ui.currentDurationLabel->setText(QString("%1ms").arg(t.msDuration));

        ProjectManager &pm = ProjectManager::instance();
        pm.setCurrentTransition(t.id);
      }
      break;
    }
  }
}

void FrameFlow::onGoToLive()
{
  RendererManager& rm = RendererManager::instance();
  EStreamingState state = rm.stremingState();
  if(state == EStreamingState::SS_STREAMING)
  {
    rm.stopStreaming();
  }
  else if(state == EStreamingState::SS_NONE)
  {
    rm.startStreaming();
  }
}

void FrameFlow::onStreamingStateChange(EStreamingState newState)
{
  if(newState == EStreamingState::SS_NONE)
  {
    ui.gotoLiveButton->setStyleSheet("QPushButton { background: #19BDDE; color: black; border: 1px solid #19BDDE; border-radius: 8px;} QPushButton:hover { background-color: #14A8C6; color: black;}");
    ui.gotoLiveButton->setText("Go Live");
    ui.liveStatusLabel->setText("Offline");
    ui.liveStatusLabel->setStyleSheet("QLabel { color: #22C35D }");
    ui.iconStatusLabel->setPixmap(QPixmap(":/FrameFlow/offline.svg"));
    streamingTimer_.invalidate();
    ui.streamTimeWidget->hide();
    ui.viewersWidget->hide();
    ui.streamingStatusLabel->setText("Offline");
    ui.streamingStatusLabel->setStyleSheet("QLabel{ background: black; color: #7B899D; border-radius: 12px; }");
    ui.streamingTimeLabel->hide();
  }
  else if(newState == EStreamingState::SS_WAITING_START)
  {
    ui.gotoLiveButton->setStyleSheet("QPushButton { background: #19BDDE; color: black; border: 1px solid #19BDDE; border-radius: 8px;} QPushButton:hover { background-color: #14A8C6; color: black;}");
    ui.gotoLiveButton->setText("Waiting Start");
    ui.liveStatusLabel->setText("Connecting");
    ui.liveStatusLabel->setStyleSheet("QLabel { color: #C39337 }");
    ui.iconStatusLabel->setPixmap(QPixmap(":/FrameFlow/offline.svg"));
    streamingTimer_.invalidate();
    ui.streamTimeWidget->hide();
    ui.viewersWidget->hide();
    ui.streamingStatusLabel->setText("Connecting");
    ui.streamingStatusLabel->setStyleSheet("QLabel{ background: black; color: #7B899D; border-radius: 12px; }");
    ui.streamingTimeLabel->hide();
  }
  else if(newState == EStreamingState::SS_STREAMING)
  {
    ui.gotoLiveButton->setStyleSheet("QPushButton { background: #DB3F40; color: white; border: 1px solid #DB3F40; border-radius: 8px;} QPushButton:hover { background-color: #C43839; color: white;}");
    ui.gotoLiveButton->setText("End Stream");
    ui.liveStatusLabel->setText("Connected");
    ui.liveStatusLabel->setStyleSheet("QLabel { color: #22C35D }");
    ui.iconStatusLabel->setPixmap(QPixmap(":/FrameFlow/wifi.svg"));
    streamingTimer_.start();
    ui.streamTimeWidget->show();
    ui.viewersWidget->show();
    ui.streamingStatusLabel->setText("LIVE");
    ui.streamingStatusLabel->setStyleSheet("QLabel{ background: #DB3F40; color: white; border-radius: 12px; }");
    ui.streamingTimeLabel->show();

    NotificationManager& nm = NotificationManager::instance();
    Notification n = { QUuid::createUuid(), "Streaming Started", "Streaming Started Description", ENotificationSeverity::S_INFO };
    nm.registerNotification(n);
  }
  else if(newState == EStreamingState::SS_WAITING_NONE)
  {
    ui.gotoLiveButton->setStyleSheet("QPushButton { background: #19BDDE; color: black; border: 1px solid #19BDDE; border-radius: 8px;} QPushButton:hover { background-color: #14A8C6; color: black;}");
    ui.gotoLiveButton->setText("Waiting Stop");
    ui.liveStatusLabel->setText("Disconnecting");
    ui.liveStatusLabel->setStyleSheet("QLabel { color: #C39337 }");
    ui.iconStatusLabel->setPixmap(QPixmap(":/FrameFlow/offline.svg"));
    streamingTimer_.invalidate();
    ui.streamTimeWidget->hide();
    ui.viewersWidget->hide();
    ui.streamingStatusLabel->setText("Disconnecting");
    ui.streamingStatusLabel->setStyleSheet("QLabel{ background: black; color: #7B899D; border-radius: 12px; }");
    ui.streamingTimeLabel->hide();
  }
}

void FrameFlow::onStartRecording()
{
  RendererManager& rm = RendererManager::instance();
  ERecordingState state = rm.recordingState();
  if(state == ERecordingState::RS_RECORDING)
  {
    rm.stopRecording();
  }
  else if(state == ERecordingState::RS_NONE)
  {
    rm.startRecording();
  }
}

void FrameFlow::onRecordingStateChange(ERecordingState newState)
{
 if(newState == ERecordingState::RS_NONE)
  {
    ui.startRecordingButton->setStyleSheet(" QPushButton { background: #171B22; color: white; border: 1px solid #647081; border-radius: 8px; } QPushButton:hover { background-color: #1E2330; color: white; }");
    ui.startRecordingButton->setText("Start Recording");
    ui.recordingStateWidget->hide();
    recordingTimer_.invalidate();
  }
  else if(newState == ERecordingState::RS_WAITING_START)
  {
    ui.startRecordingButton->setStyleSheet(" QPushButton { background: #171B22; color: white; border: 1px solid #647081; border-radius: 8px; } QPushButton:hover { background-color: #1E2330; color: white; }");
    ui.startRecordingButton->setText("Waiting Start");
    ui.recordingStateWidget->hide();
    recordingTimer_.invalidate();
  }
  else if(newState == ERecordingState::RS_RECORDING)
  {
    ui.startRecordingButton->setStyleSheet(" QPushButton { background: #3F1D23; color: #8D2226; border: 1px solid #8D2226; border-radius: 8px; } QPushButton:hover { background-color: #3F1D23; color: white; }");
    ui.startRecordingButton->setText("Stop Recording");
    ui.recordingStateWidget->show();
    recordingTimer_.start();

    NotificationManager &nm = NotificationManager::instance();
    Notification n = { QUuid::createUuid(), "Recording Started", "Recording Started Description", ENotificationSeverity::S_INFO};
    nm.registerNotification(n);
  }
  else if(newState == EStreamingState::SS_WAITING_NONE)
  {
    ui.startRecordingButton->setStyleSheet(" QPushButton { background: #171B22; color: white; border: 1px solid #647081; border-radius: 8px; } QPushButton:hover { background-color: #1E2330; color: white; }");
    ui.startRecordingButton->setText("Waiting Stop");
    ui.recordingStateWidget->hide();
    recordingTimer_.invalidate();
  }
}

void FrameFlow::updateNotifications()
{
  NotificationManager &nm = NotificationManager::instance();
  auto nl = nm.notificationList();
  if(nl.size() == 0)
  {
    ui.alertButton->setHasAlert(false);
  }
  else
  {
    ENotificationSeverity severity = ENotificationSeverity::S_INFO;
    for(auto notification : nl)
    {
      severity = std::max<ENotificationSeverity>(notification.severity, severity);
    }
    ui.alertButton->setHasAlert(true, severity);
  }
}

void FrameFlow::initExternalAudio()
{
  RendererManager& rm = RendererManager::instance();
  QStringList sl = rm.listOfAudioDevices();

  QVBoxLayout*layout = static_cast<QVBoxLayout*>(ui.scrollAreaVumetersWidget->layout());
  for(int i = 0; i < sl.size(); ++i)
  {
    VumeterControl* item = new VumeterControl;
    item->setFixedHeight(25);
    item->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    item->setDevice(sl[i]);
    layout->addWidget(item);
    connect(item, &VumeterControl::onVumeterValueChanged, this, [this, i] (double value) { RendererManager::instance().updateVolume(i, value); });
    vumeters_.push_back(item);
  }
  layout->addStretch();
}

void FrameFlow::onShowTimeline()
{
  if(ui.timelineButton->isChecked())
  {
    ui.timelineWidget->hide();
  }
  else
  {
    ui.timelineWidget->show();
  }
}
