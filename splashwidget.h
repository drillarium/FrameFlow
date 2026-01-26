#pragma once

#include <QWidget>
#include "ui_splashwidget.h"

class SplashWidget : public QWidget
{
Q_OBJECT

public:
  explicit SplashWidget(QWidget* parent = nullptr);
  void setMessage(const QString& text, int step, int totalSteps);

private:
  Ui::SplashWidgetClass ui;
};
