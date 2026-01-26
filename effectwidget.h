#pragma once

#include <QWidget>
#include "ui_effectwidget.h"

class EffectWidget : public QWidget
{
Q_OBJECT

public:
  EffectWidget(QWidget *parent = nullptr);
  ~EffectWidget();

  void setSelected(bool _selected);

private:
  Ui::EffectWidgetClass ui;
};

