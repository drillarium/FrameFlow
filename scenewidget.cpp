#include "scenewidget.h"
#include <QStyle>

SceneWidget::SceneWidget(QWidget *_parent)
:QWidget(_parent)
{
  ui.setupUi(this);
  ui.menuButton->hide();
}

SceneWidget::~SceneWidget()
{
}

void SceneWidget::enterEvent(QEnterEvent*)
{
  ui.menuButton->show();
}

void SceneWidget::leaveEvent(QEvent*)
{
  ui.menuButton->hide();
}

void SceneWidget::setSelected(bool _selected)
{
  QString ss = "#sceneTitleLabel {\
    color: white;\
  }\
  \
  #mainSceneWidget {\
    background: #171B22;\
    border: 2px solid %1;\
    border-radius: 8px;\
  }\
  \
  #mainSceneWidget:hover {\
    background: #1E2430;\
    color: white;\
    border: 2px solid %1;\
    border-radius: 8px;\
  }\
  \
  #statusLabel {\
    color: green;\
  }\
  \
  #centerWidget {\
    border: 2px solid transparent;\
   border-radius: 12px;\
    background: qlineargradient(\
      x1:0, y1:0,\
      x2:1, y2:1,\
      stop:0 #ff3b3b,\
      stop:1 #8b0000\
    );\
  }";

  QString color = _selected? "#19BDDE" : "transparent";
  setStyleSheet(QString(ss).arg(color));
  style()->unpolish(this);
  style()->polish(this);
  update();
}
