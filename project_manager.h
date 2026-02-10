#pragma once

#include <QObject>
#include <QVector>
#include <optional>
#include "project_model.h"
#include <QRect>

class ProjectManager : public QObject
{
  Q_OBJECT

public:
  enum WorkingMode { CONT, STUDIO };
  static ProjectManager& instance();

  bool openDatabase(const QString& dbPath);

  // Project lifecycle
  bool createProject(const QString& name);
  bool createProject(Project& project);
  bool loadProject(const QUuid& projectId);
  bool saveCurrentProject();
  bool addScene(Scene &_scene);
  bool removeScene(const QUuid& sceneId);
  bool renameScene(const QUuid& sceneId, const QString &_newName);
  bool saveProject(Project &_project);
  void closeProject();
  bool deleteProject(const QUuid& projectId);
  void setCurrentScene(const QUuid &_id);
  QUuid currentSceneId() { return currentSceneId_; }
  bool addSource(Source &_source);
  bool removeSource(const QUuid& sourceId);
  void setCurrentSource(const QUuid& _id) { currentSourceId_ = _id; }
  QUuid currentSourceId() { return currentSourceId_; }
  bool updateSource(const Source &_source);
  bool updateSourceRect(const QRect&_r);
  void setWorkingMode(ProjectManager::WorkingMode _mode) { mode_ = _mode; }
  ProjectManager::WorkingMode workingMode() { return mode_; }
  void setCurrentStudioSceneId(QUuid id) { studioSceneId_ = id; }
  QUuid currentStudioSceneId() { return studioSceneId_; }
  bool moveSource(const QUuid &source, bool up);

  // Queries
  QVector<Project> listProjects() const;
  std::optional<Project> currentProject() const { return currentProject_; }

  bool isDirty() const;
  void setDirty(bool dirty = true);

signals:
  void currentProjectChanged();
  void projectListChanged();
  void dirtyChanged(bool dirty);

private:
  explicit ProjectManager(QObject* parent = nullptr);
  ~ProjectManager() = default;

  Q_DISABLE_COPY_MOVE(ProjectManager)

private:
  std::optional<Project> currentProject_;
  QVector<Project> cachedProjects_;
  bool dirty_ = false;
  QUuid currentSceneId_;
  QUuid currentSourceId_;
  QUuid studioSceneId_;
  ProjectManager::WorkingMode mode_ = WorkingMode::CONT;
};
