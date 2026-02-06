#include "sourcewidget.h"
#include <QStyle>
#include <QMenu>

SourceWidget::SourceWidget(const Source& _source, QWidget *_parent)
:QWidget(_parent)
,source_(_source)
{
  ui.setupUi(this);

  ui.lockButton->hide();
  ui.eyeButton->hide();
  ui.menuButton->hide();
  ui.titleLabel->setText(source_.name);

  QMenu* menu = new QMenu(ui.menuButton);
  menu->setCursor(Qt::PointingHandCursor);
  QAction* renameAction = menu->addAction(QIcon(":/FrameFlow/pencilwhite.svg"), "Edit");
  QAction* deleteAction = menu->addAction(QIcon(":/FrameFlow/trash.svg"), "Delete");
  ui.menuButton->setMenu(menu);

  connect(renameAction, &QAction::triggered, this, &SourceWidget::onRenameSource);
  connect(deleteAction, &QAction::triggered, this, &SourceWidget::onDeleteSource);
}

SourceWidget::~SourceWidget()
{
}

void SourceWidget::enterEvent(QEnterEvent*)
{
  ui.lockButton->show();
  ui.eyeButton->show();
  ui.menuButton->show();
}

void SourceWidget::leaveEvent(QEvent*)
{
  ui.lockButton->hide();
  ui.eyeButton->hide();
  ui.menuButton->hide();
}

void SourceWidget::setSelected(bool _selected)
{
  QString ss = "#mainSourceWidget {\
    background: #171B22;\
    border: 2px solid %1;\
    border-radius: 8px;\
  }\
  \
  #mainSourceWidget:hover {\
    background: #1E2430;\
    border: 2px solid %1;\
    border-radius: 8px;\
  }\
  \
  #titleLabel {\
    color: white;\
  }";

  QString color = _selected? "#19BDDE" : "transparent";
  setStyleSheet(QString(ss).arg(color));
  style()->unpolish(this);
  style()->polish(this);
  update();
}
