#pragma once

#include <QWidget>
#include "ui_colorpickerwidget.h"
#include "basesourcewidget.h"

class ColorPickerWidget : public BaseSourceWidget
{
Q_OBJECT

public:
  ColorPickerWidget(QWidget *parent = nullptr);
  ~ColorPickerWidget();

  void init() override { }
  SourceType type() override { return SourceType::EST_COLOR; }
  int h() override { return 125; }
  Source source() override;
  bool isValid() override;
  void editSource(const Source& _source) override;

protected slots:
  void onPickColor();

private:
  Ui::ColorPickerWidgetClass ui;
};

