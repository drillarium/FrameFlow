#include "alertwidget.h"
#include <QStyle>

AlertWidget::AlertWidget(QWidget *parent)
:QWidget(parent)
{
  ui.setupUi(this);
  ui.closeAlertButton->hide();
  connect(ui.closeAlertButton, &QPushButton::clicked, this, &AlertWidget::onRemoveAlert);
}

AlertWidget::~AlertWidget()
{

}

void AlertWidget::enterEvent(QEnterEvent* event)
{
  ui.closeAlertButton->show();
  QWidget::enterEvent(event);
}

void AlertWidget::leaveEvent(QEvent* event)
{
  ui.closeAlertButton->hide();
  QWidget::leaveEvent(event);
}

QString timeAgo(const QDateTime& dateTime)
{
  qint64 seconds = dateTime.secsTo(QDateTime::currentDateTime());

  if(seconds < 0) return "in the future";

  if(seconds < 60) return "just now";

  qint64 minutes = seconds / 60;
  if(minutes < 60) return QString("%1 min ago").arg(minutes);

  qint64 hours = minutes / 60;
  if(hours < 24) return QString("%1 hour%2 ago").arg(hours).arg(hours > 1 ? "s" : "");

  qint64 days = hours / 24;
  if(days == 1) return "yesterday";

  if(days < 7) return QString("%1 days ago").arg(days);

  return dateTime.toString("yyyy-MM-dd");
}

void AlertWidget::update(const Notification& _notification)
{
  id_ = _notification.id;
  QString text = timeAgo(_notification.modificationTime);
  ui.titleLabel->setText(_notification.title);
  ui.descriptionLabel->setText(_notification.desciption);
  ui.elapsedLabel->setText(text);
  ui.iconLabel->setPixmap(QPixmap(":/FrameFlow/check.svg"));
}

void AlertWidget::setSelected(bool _selected)
{
  QString ss = "#mainAlertWidget {\
  background: #1C1F26;\
  border: 1px solid %1;\
  border-radius: 12px;\
}\
\
QLabel {\
  color: grey;\
}\
\
#titleLabel {\
  color: white;\
}\
\
#closeAlertButton {\
  border: 0px solid black;\
}";

  QString color = _selected ? "white" : "transparent";
  setStyleSheet(QString(ss).arg(color));
  style()->unpolish(this);
  style()->polish(this);
  QWidget::update();
}
