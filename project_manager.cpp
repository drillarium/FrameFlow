#include "project_manager.h"
#include "db.h"

#include <QDateTime>

ProjectManager& ProjectManager::instance()
{
  static ProjectManager instance;
  return instance;
}

ProjectManager::ProjectManager(QObject* parent)
  : QObject(parent)
{
}

bool ProjectManager::openDatabase(const QString& dbPath)
{
  if(!Database::instance().open(dbPath))
    return false;

  cachedProjects_ = Database::instance().listProjects();
  emit projectListChanged();

  return true;
}

QVector<Project> ProjectManager::listProjects() const
{
  return cachedProjects_;
}

bool ProjectManager::createProject(const QString& name)
{
  Project p;
  p.id = QUuid::createUuid();
  p.name = name;
  p.createdAt = QDateTime::currentDateTimeUtc();
  p.modifiedAt = p.createdAt;

  if(!Database::instance().saveProject(p))
    return false;

  currentProject_ = p;
  cachedProjects_ = Database::instance().listProjects();
  dirty_ = false;

  emit currentProjectChanged();
  emit projectListChanged();
  emit dirtyChanged(false);

  return true;
}

bool ProjectManager::loadProject(const QUuid& projectId)
{
  auto project = Database::instance().loadProject(projectId);
  if(!project)
    return false;

  currentProject_ = *project;
  dirty_ = false;

  emit currentProjectChanged();
  emit dirtyChanged(false);

  return true;
}

bool ProjectManager::saveCurrentProject()
{
  if(!currentProject_)
    return false;

  currentProject_->modifiedAt = QDateTime::currentDateTimeUtc();

  if(!Database::instance().saveProject(*currentProject_))
    return false;

  dirty_ = false;
  cachedProjects_ = Database::instance().listProjects();

  emit dirtyChanged(false);
  emit projectListChanged();

  return true;
}

bool ProjectManager::deleteProject(const QUuid& projectId)
{
  if(!Database::instance().deleteProject(projectId))
    return false;

  if(currentProject_ && currentProject_->id == projectId) {
    closeProject();
  }

  cachedProjects_ = Database::instance().listProjects();
  emit projectListChanged();

  return true;
}

void ProjectManager::closeProject()
{
  if(!currentProject_)
    return;

  currentProject_.reset();
  dirty_ = false;

  emit currentProjectChanged();
  emit dirtyChanged(false);
}

bool ProjectManager::isDirty() const
{
  return dirty_;
}

void ProjectManager::setDirty(bool dirty)
{
  if(dirty_ == dirty)
    return;

  dirty_ = dirty;
  emit dirtyChanged(dirty_);
}

