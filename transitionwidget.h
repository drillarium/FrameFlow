#pragma once

#include <QWidget>
#include "ui_transitionwidget.h"

class TransitionWidget : public QWidget
{
Q_OBJECT

public:
  TransitionWidget(QWidget *parent = nullptr);
  ~TransitionWidget();
  void setSelected(bool _selected);

private:
  Ui::TransitionWidgetClass ui;
};

