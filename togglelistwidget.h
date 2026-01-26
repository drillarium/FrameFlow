#pragma once

#include <QListWidget>
#include <QMouseEvent>

class ToggleListWidget : public QListWidget
{
  Q_OBJECT
public:
  using QListWidget::QListWidget;

protected:
  void mousePressEvent(QMouseEvent* e) override
  {
    QListWidgetItem* item = itemAt(e->pos());

    if(item && item->isSelected()) {
      clearSelection();
      setCurrentItem(nullptr);
      return;
    }

    QListWidget::mousePressEvent(e);
  }
};

