#include "projectmenuitem.h"

ProjectMenuItem::ProjectMenuItem(QWidget *_parent)
:QWidget(_parent)
{
  ui.setupUi(this);
}

ProjectMenuItem::~ProjectMenuItem()
{

}

void ProjectMenuItem::onStartEdit()
{
  ui.stackedWidget->setCurrentIndex(1);
}

void ProjectMenuItem::onEndEdit()
{
  ui.stackedWidget->setCurrentIndex(0);
}