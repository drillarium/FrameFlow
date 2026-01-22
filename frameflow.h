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

protected slots:
  void updateSystemStats();
  void onAlerts();
  void onSettings();
  void onSetWindowTitleVisible();

private:
  void readSettings();
  void writeSettings();

private:
  Ui::FrameFlowClass ui;
  class ProjectsWidget* projectsWidget_;
  bool windowTitleVisible_ = true;
};

