#pragma once

#include <QWidget>
#include "ui_transitionwidget.h"
#include "transition_model.h"

class TransitionWidget : public QWidget
{
Q_OBJECT

public:
  TransitionWidget(const Transition &_transition, QWidget *parent = nullptr);
  ~TransitionWidget();
  void setSelected(bool _selected);
  Transition transition() { return transition_; }
  void setTransition(const Transition& _transition);

private:
  Ui::TransitionWidgetClass ui;
  Transition transition_;
};

