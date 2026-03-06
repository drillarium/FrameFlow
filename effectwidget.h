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

protected:
  void enterEvent(QEnterEvent*) override;
  void leaveEvent(QEvent*) override;

private:
  Ui::EffectWidgetClass ui;
};

