#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_frameflow.h"
#include "scene_model.h"
#include "license_manager.h"
#include "renderermanager.h"

class FrameFlow : public QMainWindow
{
  Q_OBJECT

public:
  FrameFlow(QWidget *_parent = nullptr);
  ~FrameFlow();

protected:
  void closeEvent(QCloseEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;
  void selectScene(QUuid scene);

protected slots:
  void updateSystemStats();
  void onAlerts();
  void onSettings();
  void onSetWindowTitleVisible();
  void onExpandTransitions();
  void onExpandEffects();
  void onSceneSelectionChanged();
  void onSourceSelectionChanged();
  void onTransitionSelectionChanged();
  void onEffectSelectionChanged();
  void onAddSource();
  void onCurrentProjectChange();
  void onProjectListChanged();
  void onCreateScene();
  void onUpdateNumSources();
  void onUpdateNumScenes();
  void onCurrentSourceRectChange(const QRect& _r);
  void onTake();
  void onLicenseChanged(LicenseManager::Status newStatus);
  void onHelp();
  void onToggleFullScreen();
  void onChangeTransitionDuration();
  void onChangeTransitionSelected();
  void onStreamingStateChange(EStreamingState _newSate);
  void onGoToLive();
  void onStartRecording();
  void onRecordingStateChange(ERecordingState newState);
  void updateNotifications();
  void updateProjectInfo();
  void onShowTimeline();

private:
  void readSettings();
  void writeSettings();
  void updateScenes();
  void deleteScene(const Scene &_scene);
  void renameScene(const Scene &_scene);
  void updateSources();
  void deleteSource(const Source& _source);
  void renameSource(const Source& _source);
  void moveSource(const Source& _source, bool up);
  void showLicenseStatus(LicenseManager::Status status);
  void initExternalAudio();

private:
  Ui::FrameFlowClass ui;
  class ProjectsWidget* projectsWidget_;
  bool windowTitleVisible_ = true;
  QUuid nextProjectUID_;
  bool fullScreen_ = false;
  QUuid nextTransitionUID_;
  QElapsedTimer streamingTimer_;
  QElapsedTimer recordingTimer_;
  QList<class VumeterControl*> vumeters_;
};

