#pragma once

#include <QWidget>

class WidgetButton  : public QWidget
{
Q_OBJECT

public:
  WidgetButton(QWidget *parent);
  ~WidgetButton();

signals:
  void clicked();

protected:
  void mousePressEvent(QMouseEvent*) override;
  void mouseReleaseEvent(QMouseEvent*) override;
  void enterEvent(QEnterEvent*) override;
  void leaveEvent(QEvent*);

protected:
  bool pressed_ = false;
  bool hovered_ = false;
};

