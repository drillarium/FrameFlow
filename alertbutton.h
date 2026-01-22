#pragma once

#include <QPushButton>

class AlertButton  : public QPushButton
{
    Q_OBJECT

public:
  AlertButton(QWidget *parent);
  ~AlertButton();

  void setHasAlert(bool value);

protected:
  void paintEvent(QPaintEvent* e) override;

protected:
  bool hasAlert_ = true;
};

