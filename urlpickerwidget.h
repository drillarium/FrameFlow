#pragma once

#include <QWidget>
#include "ui_urlpickerwidget.h"
#include "basesourcewidget.h"

class UrlPickerWidget : public BaseSourceWidget
{
Q_OBJECT

public:
  UrlPickerWidget(QWidget *parent = nullptr);
  ~UrlPickerWidget();

  SourceType type() override { return SourceType::EST_URL; }
  int h() override { return 125; }
  Source source();

private:
  Ui::UrlPickerWidgetClass ui;
};

