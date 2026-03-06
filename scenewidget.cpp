#include "scenewidget.h"
#include <QStyle>
#include <QMenu>
#include "project_manager.h"

const QString from[] = {
    "#203A5E", // Deep Blue
    "#225E5E", // Soft Teal
    "#2D506E", // Cyan Slate
    "#3C6E96", // Arctic Blue

    "#28553C", // Forest
    "#1E6E46", // Emerald Dark
    "#555F2D", // Olive Slate
    "#32785A", // Mint Deep

    "#373C78", // Deep Indigo
    "#554178", // Muted Violet
    "#5F325F", // Plum Dark
    "#462D82", // Royal Purple

    "#784628", // Burnt Orange
    "#8C5537", // Copper
    "#78282D", // Deep Red
    "#6E233C", // Wine

    "#3C4B5F", // Slate Blue Grey
    "#465055", // Cool Grey
    "#37414B", // Charcoal Lift
    "#32465A"  // Graphite Blue
};

const QString to[] = {
    "#122340",
    "#143C3C",
    "#19324B",
    "#234664",

    "#193728",
    "#144B2D",
    "#37411E",
    "#1E503C",

    "#1E2350",
    "#372855",
    "#3C1E3C",
    "#2D195A",

    "#502D19",
    "#5A3723",
    "#50191E",
    "#4B1428",

    "#233246",
    "#2D373C",
    "#232D37",
    "#1E2D41"
};

SceneWidget::SceneWidget(Scene& _scene, QWidget *_parent)
:QWidget(_parent)
,scene_(_scene)
{
  ui.setupUi(this);
  ui.menuButton->hide();
  ui.sceneTitleLabel->setText(_scene.name);

  QMenu* menu = new QMenu(ui.menuButton);
  menu->setCursor(Qt::PointingHandCursor);
  QAction* moveUpAction = menu->addAction(QIcon(":/FrameFlow/up.svg"), "Move up");
  QAction* moveDownAction = menu->addAction(QIcon(":/FrameFlow/down.svg"), "Move down");
  QAction* renameAction = menu->addAction(QIcon(":/FrameFlow/pencilwhite.svg"), "Rename");
  QAction *deleteAction = menu->addAction(QIcon(":/FrameFlow/trash.svg"), "Delete");
  ui.menuButton->setMenu(menu);

  connect(renameAction, &QAction::triggered, this, &SceneWidget::onRenameScene);
  connect(deleteAction, &QAction::triggered, this, &SceneWidget::onDeleteScene);
  connect(moveUpAction, &QAction::triggered, this, &SceneWidget::onUpScene);
  connect(moveDownAction, &QAction::triggered, this, &SceneWidget::onDownScene);
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
    color: %1;\
  }\
  #centerWidget {\
    border: 2px solid transparent;\
   border-radius: 12px;\
    background: qlineargradient(\
      x1:0, y1:0,\
      x2:1, y2:1,\
      stop:0 %2,\
      stop:1 %3\
    );\
  }\
    QMenu{\
      background-color: #2b2b2b;\
      color: #ffffff;\
      border: 1px solid #444;\
      padding: 4px;\
  }\
  QMenu::item { background-color: transparent; }\
  QMenu::item:selected { background-color: #303541; }\
  #menuButton { background: #66000000; border: 1px solid transparent; border-radius: 3px; }\
  #menuButton::menu-indicator { image: none; width: 0px; }\
  #centerLabel { color: #22ffffff; }\
  #menuButton:hover { background: #99000000; }";

  QString color = _selected? "#19BDDE" : "transparent";
  setStyleSheet(QString(ss).arg(color).arg(from[scene_.colorIndex]).arg(to[scene_.colorIndex]));
  style()->unpolish(this);
  style()->polish(this);
  update();

  if(_selected) ui.statusLabel->show();
  else ui.statusLabel->hide();
}
