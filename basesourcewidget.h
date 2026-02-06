#pragma once

#include <QWidget>
#include "source_model.h"

class BaseSourceWidget : public QWidget
{
public:
  BaseSourceWidget(QWidget* _parent) : QWidget(_parent) {}
  virtual SourceType type() = 0;
  virtual int h() = 0;
  virtual Source source() = 0;
  virtual bool isValid() = 0;
  virtual void editSource(const Source &_source) = 0;
};
