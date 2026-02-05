#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_frameflow.h"
#include "scene_model.h"

class FrameFlow : public QMainWindow
{
  Q_OBJECT

public:
  FrameFlow(QWidget *_parent = nullptr);
  ~FrameFlow();

protected:
  void closeEvent(QCloseEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;

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
  void onProjectDirtyChange();
  void onCreateScene();
  void onUpdateNumSources();
  void onUpdateNumScenes();

private:
  void readSettings();
  void writeSettings();
  void updateScenes();
  void deleteScene(const Scene &_scene);
  void renameScene(const Scene &_scene);
  void updateSources();

private:
  Ui::FrameFlowClass ui;
  class ProjectsWidget* projectsWidget_;
  bool windowTitleVisible_ = true;
};

