#pragma once

#include <QWidget>
#include "ui_sourcewidget.h"
#include "source_model.h"

class SourceWidget : public QWidget
{
Q_OBJECT

public:
  SourceWidget(const Source &_source, QWidget *_parent = nullptr);
  ~SourceWidget();
  void setSelected(bool _selected);
  QUuid id() { return source_.id; }

protected:
  void enterEvent(QEnterEvent*) override;
  void leaveEvent(QEvent*) override;

signals:
  void onDeleteSource();
  void onRenameSource();

private:
  Source source_;
  Ui::SourceWidgetClass ui;
};

