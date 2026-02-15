#include "project_manager.h"
#include "db.h"
#include <QDateTime>

ProjectManager& ProjectManager::instance()
{
  static ProjectManager instance;
  return instance;
}

ProjectManager::ProjectManager(QObject* parent)
:QObject(parent)
{
}

bool ProjectManager::openDatabase(const QString& dbPath)
{
  if(!Database::instance().open(dbPath)) return false;

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
  p.name = name;

  return createProject(p);
}

bool ProjectManager::createProject(Project& project)
{
  project.id = QUuid::createUuid();
  project.createdAt = QDateTime::currentDateTimeUtc();
  project.modifiedAt = project.createdAt;
  if(project.scenes.size() == 0)
  {
    Scene scene = {};
    scene.id = QUuid::createUuid();
    scene.name = "Main Scene";
    scene.orderIndex = project.scenes.size();
    project.scenes.push_back(scene);
  }

  for(int i = 0; i < project.scenes.size(); i++)
  {
    project.scenes[i].projectId = project.id;
  }

  if(!Database::instance().saveProject(project)) return false;

  currentProject_ = project;
  cachedProjects_ = Database::instance().listProjects();

  emit currentProjectChanged();
  emit projectListChanged();

  return true;
}

bool ProjectManager::loadProject(const QUuid& projectId)
{
  auto project = Database::instance().loadProject(projectId);
  if(!project) return false;

  currentProject_ = *project;

  emit currentProjectChanged();

  return true;
}

bool ProjectManager::saveProject(Project& _project)
{
  for(int i = 0; i < cachedProjects_.size(); i++)
  {
    if(cachedProjects_[i].id == _project.id)
    {
      _project.modifiedAt = QDateTime::currentDateTimeUtc();
      if(!Database::instance().saveProject(_project)) return false;

      cachedProjects_ = Database::instance().listProjects();

      // current project
      if(currentProject_ && currentProject_->id == _project.id)
      {
        currentProject_ = _project;
        emit currentProjectChanged();
      }
      emit projectListChanged();

      return true;
    }
  }

  return false;
}

bool ProjectManager::addScene(Scene& _scene)
{
  if(!currentProject_) return false;
  
  _scene.id = QUuid::createUuid();
  _scene.projectId = currentProject_->id;
  currentProject_->scenes.push_back(_scene);

  saveCurrentProject();
  emit currentProjectChanged();

  return true;
}

bool ProjectManager::removeScene(const QUuid& sceneId)
{
  if(!currentProject_) return false;

  auto it = std::find_if(currentProject_->scenes.begin(), currentProject_->scenes.end(), [sceneId] (const Scene& s) { return s.id == sceneId; });
  if(it == currentProject_->scenes.end()) return false;
  currentProject_->scenes.erase(it);

  // normalize order index
  for(int i = 0; i < currentProject_->scenes.size(); ++i) currentProject_->scenes[i].orderIndex = i;

  Database::instance().deleteScene(sceneId);
  saveCurrentProject();

  emit currentProjectChanged();

  return true;
}

bool ProjectManager::renameScene(const QUuid& sceneId, const QString& _newName)
{
  if(!currentProject_) return false;

  auto it = std::find_if(currentProject_->scenes.begin(), currentProject_->scenes.end(), [sceneId](const Scene& s) { return s.id == sceneId; });
  if(it == currentProject_->scenes.end()) return false;
  it->name = _newName;

  saveCurrentProject();  
  emit currentProjectChanged();

  return true;
}

bool ProjectManager::saveCurrentProject()
{
  if(!currentProject_) return false;

  currentProject_->modifiedAt = QDateTime::currentDateTimeUtc();

  if(!Database::instance().saveProject(*currentProject_)) return false;

  cachedProjects_ = Database::instance().listProjects();

  emit projectListChanged();

  return true;
}

bool ProjectManager::deleteProject(const QUuid& projectId)
{
  if(!Database::instance().deleteProject(projectId)) return false;

  bool isCurrent = (currentProject_ && currentProject_->id == projectId);
  if(isCurrent)
  {
    closeProject();
  }

  cachedProjects_ = Database::instance().listProjects();
  emit projectListChanged();

  // load first one
  if(isCurrent)
  {
    if(cachedProjects_.size() > 0)
    {
      loadProject(cachedProjects_[0].id);
    }
  }

  return true;
}

void ProjectManager::closeProject()
{
  if(!currentProject_) return;

  currentProject_.reset();

  emit currentProjectChanged();
}

bool ProjectManager::addSource(Source &_source)
{
  if(!currentProject_) return false;

  for(int i = 0; i < currentProject_->scenes.size(); ++i)
  {
    if(currentProject_->scenes[i].id == currentSceneId_)
    {
      _source.id = QUuid::createUuid();
      _source.orderIndex = currentProject_->scenes[i].sources.size();
      _source.sceneId = currentProject_->scenes[i].id;
      currentProject_->scenes[i].sources.push_back(_source);

      saveCurrentProject();
      emit currentProjectChanged();
      
      return true;
    }
  }

  return false;
}

bool ProjectManager::removeSource(const QUuid& sourceId)
{
  if(!currentProject_) return false;

  for(int i = 0; i < currentProject_->scenes.size(); ++i)
  {
    auto it = std::find_if(currentProject_->scenes[i].sources.begin(), currentProject_->scenes[i].sources.end(), [sourceId](const Source& s) { return s.id == sourceId; });
    if(it != currentProject_->scenes[i].sources.end())
    {
      currentProject_->scenes[i].sources.erase(it);

      // normalize order index
      for(int j = 0; j < currentProject_->scenes[i].sources.size(); ++j) currentProject_->scenes[i].sources[j].orderIndex = j;

      Database::instance().deleteSource(sourceId);
      saveCurrentProject();
      emit currentProjectChanged();

      return true;
    }
  }

  return false;
}

bool ProjectManager::updateSource(const Source& _source)
{
  if(!currentProject_) return false;

  QUuid sourceId = _source.id;
  for(int i = 0; i < currentProject_->scenes.size(); ++i)
  {
    auto it = std::find_if(currentProject_->scenes[i].sources.begin(), currentProject_->scenes[i].sources.end(), [sourceId](const Source& s) { return s.id == sourceId; });
    if(it != currentProject_->scenes[i].sources.end())
    {
      *it = _source;
      (*it).dirty = true;

      saveCurrentProject();
      emit currentProjectChanged();

      return true;
    }
  }

  return false;
}

void ProjectManager::resetDirty()
{
  if(!currentProject_) return;

  for(int i = 0; i < currentProject_->scenes.size(); ++i)
  {
    for(int j = 0; j < currentProject_->scenes[i].sources.size(); ++j)
    {
      currentProject_->scenes[i].sources[j].dirty = false;
    }    
  }
}

bool ProjectManager::updateSourceRect(const QRect& _r)
{
  if(!currentProject_) return false;

  QUuid sourceId = currentSourceId_;
  for(int i = 0; i < currentProject_->scenes.size(); ++i)
  {
    auto it = std::find_if(currentProject_->scenes[i].sources.begin(), currentProject_->scenes[i].sources.end(), [sourceId](const Source& s) { return s.id == sourceId; });
    if(it != currentProject_->scenes[i].sources.end())
    {
      // update source rect
      setSourceRect(*it, _r);

      saveCurrentProject();
      emit currentProjectChanged();

      return true;
    }
  }

  return false;
}

void ProjectManager::setCurrentScene(const QUuid& _id)
{
  currentSceneId_ = _id;
  if(mode_ == WorkingMode::CONT)
  {
    studioSceneId_ = currentSceneId_;
  }
}

bool ProjectManager::moveSource(const QUuid& sourceId, bool up)
{
  if(!currentProject_) return false;

  for(int i = 0; i < currentProject_->scenes.size(); ++i)
  {
    auto it = std::find_if(currentProject_->scenes[i].sources.begin(), currentProject_->scenes[i].sources.end(), [sourceId](const Source& s) { return s.id == sourceId; });
    if(it != currentProject_->scenes[i].sources.end())
    {
      bool save = false;
      if(up && it != currentProject_->scenes[i].sources.begin())
      {
        std::iter_swap(it, it - 1);
        save = true;
      }
      else if(!up && it + 1 != currentProject_->scenes[i].sources.end())
      {
        std::iter_swap(it, it + 1);
        save = true;
      }
      
      if(save)
      {
        // normalize order index
        for(int j = 0; j < currentProject_->scenes[i].sources.size(); ++j) currentProject_->scenes[i].sources[j].orderIndex = j;

        saveCurrentProject();
        emit currentProjectChanged();
      }

      return true;
    }
  }

  return false;
}