#pragma once

#include <QWidget>
#include "ui_textwidget.h"
#include "basesourcewidget.h"

class TextWidget : public BaseSourceWidget
{
Q_OBJECT

public:
  TextWidget(QWidget *parent = nullptr);
  ~TextWidget();

  void init() override { }
  SourceType type() override { return SourceType::EST_TEXT; }
  int h() override { return 280; }
  Source source() override;
  bool isValid() override;
  void editSource(const Source& _source) override;

protected slots:
  void onPickColor();

private:
  Ui::TextWidgetClass ui;
};

