#pragma once

#include <QWidget>
#include "ui_pathpickerwidget.h"
#include "basesourcewidget.h"

class PathPickerWidget : public BaseSourceWidget
{
Q_OBJECT

public:
  PathPickerWidget(QWidget *parent = nullptr);
  ~PathPickerWidget();

  SourceType type() override { return SourceType::EST_FILE; }
  int h() override { return 125; }
  Source source();

protected slots:
  void onPickPath();

private:
  Ui::PathPickerWidgetClass ui;
};

