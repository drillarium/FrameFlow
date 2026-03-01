#pragma once

#include <QWidget>

// VuMeterWidget
class VuMeterWidget : public QWidget
{
  Q_OBJECT
public:
  explicit VuMeterWidget(QWidget* parent = nullptr);

  void setLevelDb(float db);   // e.g. -60 .. 0

protected:
  void paintEvent(QPaintEvent*) override;
  void drawHeader(QPainter &_p);
  void drawBar(QPainter& _p);

private:
  float m_db = -60.0f;
  int headerHeight_ = 10;
};