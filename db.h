#pragma once

#include <QSqlDatabase>
#include <QMutex>
#include <optional>
#include "project_model.h"
#include "transition_model.h"
#include "server_model.h"

class Database
{
public:
  static Database& instance();

  bool open(const QString& filePath);
  void close();
  bool isOpen() const;

  // Schema
  bool createSchemaIfNeeded();

  // Projects
  bool saveProject(const Project& project);
  std::optional<Project> loadProject(const QUuid& projectId);
  bool deleteProject(const QUuid& projectId);
  QVector<Project> listProjects();
  bool deleteScene(const QUuid& sceneId);
  bool deleteSource(const QUuid& sourceId);
  QVector<Transition> listTransitions();
  QVector<StreamingServer> listStreamingServers();
  bool saveStreamingServer(const StreamingServer &_streamingServer);
  bool removeStreamingServer(const QUuid &_id);
  bool removeStreamingServers();

private:
  Database();
  ~Database();

  Q_DISABLE_COPY_MOVE(Database)

  bool execSchemaStatements(const QStringList& statements);
  bool setSchemaVersion(int version);
  int  schemaVersion() const;
  QStringList schemaV1Statements() const;

  bool migrateV1ToV2();
  bool migrateV2ToV3();
  bool migrateV3ToV4();
  bool migrateV4ToV5();
  bool migrateV5ToV6();

private:
  QSqlDatabase db_;
  mutable QMutex mutex_;
};
