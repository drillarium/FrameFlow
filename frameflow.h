#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_frameflow.h"

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
  void onSceneSelectedChange(QListWidgetItem*, QListWidgetItem*);
  void onSourceSelectedChange(QListWidgetItem*, QListWidgetItem*);
  void onTransitionSelectedChange(QListWidgetItem*, QListWidgetItem*);
  void onEffectSelectedChange(QListWidgetItem*, QListWidgetItem*);
  void onExpandTransitions();
  void onExpandEffects();

private:
  void readSettings();
  void writeSettings();

private:
  Ui::FrameFlowClass ui;
  class ProjectsWidget* projectsWidget_;
  bool windowTitleVisible_ = true;
};

