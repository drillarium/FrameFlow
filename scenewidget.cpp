#include "scenewidget.h"
#include <QStyle>
#include <QMenu>
#include "project_manager.h"

SceneWidget::SceneWidget(Scene& _scene, QWidget *_parent)
:QWidget(_parent)
,scene_(_scene)
{
  ui.setupUi(this);
  ui.menuButton->hide();
  ui.sceneTitleLabel->setText(_scene.name);

  QMenu* menu = new QMenu(ui.menuButton);
  menu->setCursor(Qt::PointingHandCursor);
  QAction* renameAction = menu->addAction(QIcon(":/FrameFlow/pencilwhite.svg"), "Rename");
  QAction *deleteAction = menu->addAction(QIcon(":/FrameFlow/trash.svg"), "Delete");
  ui.menuButton->setMenu(menu);

  connect(renameAction, &QAction::triggered, this, &SceneWidget::onRenameScene);
  connect(deleteAction, &QAction::triggered, this, &SceneWidget::onDeleteScene);
  connect(menu, &QMenu::aboutToShow, this, [&, deleteAction](){ 
    ProjectManager& pm = ProjectManager::instance();
    auto project = pm.currentProject();
    if(!project) return;
    deleteAction->setEnabled(project->scenes.size() > 1);
  });
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
  #mainSceneWidget {\
    background: #171B22;\
    border: 2px solid %1;\
    border-radius: 8px;\
  }\
  #mainSceneWidget:hover {\
    background: #1E2430;\
    color: white;\
    border: 2px solid %1;\
    border-radius: 8px;\
  }\
  #statusLabel {\
    color: green;\
  }\
  #centerWidget {\
    border: 2px solid transparent;\
   border-radius: 12px;\
    background: qlineargradient(\
      x1:0, y1:0,\
      x2:1, y2:1,\
      stop:0 #1B5E69,\
      stop:1 #2A404B\
    );\
  }\
    QMenu{\
      background-color: #2b2b2b;\
      color: #ffffff;\
      border: 1px solid #444;\
      padding: 4px;\
  }\
  QMenu::item {\
      background-color: transparent;\
  }\
  QMenu::item:selected {\
      background-color: #303541;\
  }\
  ";

  QString color = _selected? "#19BDDE" : "transparent";
  setStyleSheet(QString(ss).arg(color));
  style()->unpolish(this);
  style()->polish(this);
  update();

  if(_selected) ui.statusLabel->show();
  else ui.statusLabel->hide();
}
