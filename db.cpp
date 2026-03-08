#include "db.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QJsonDocument>
#include <QDebug>

static constexpr int CURRENT_SCHEMA_VERSION = 6;

Database& Database::instance()
{
  static Database instance;
  return instance;
}

Database::Database()
{
}

Database::~Database()
{
  close();
}

bool Database::open(const QString& filePath)
{
  QMutexLocker lock(&mutex_);

  if(db_.isOpen()) return true;

  db_ = QSqlDatabase::addDatabase("QSQLITE");
  db_.setDatabaseName(filePath);

  if(!db_.open())
  {
    qCritical() << "DB open failed:" << db_.lastError();
    return false;
  }

  QSqlQuery q;
  q.exec("PRAGMA foreign_keys = ON;");

  return createSchemaIfNeeded();
}

void Database::close()
{
  QMutexLocker lock(&mutex_);

  if(!db_.isOpen()) return;

  db_.close();
  QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
}

bool Database::isOpen() const
{
  return db_.isOpen();
}
bool Database::createSchemaIfNeeded()
{
  QSqlQuery q(db_);

  // Does meta table exist?
  if(!q.exec("SELECT name FROM sqlite_master WHERE type='table' AND name='meta'"))
  {
    qCritical() << q.lastError();
    return false;
  }

  if(!q.next())
  {
    // Fresh DB => create schema v1
    if(!execSchemaStatements(schemaV1Statements())) return false;

    if(!setSchemaVersion(1)) return false;
  }

  int version = schemaVersion();
  if(version < 0) return false;

  // Incremental migrations
  while(version < CURRENT_SCHEMA_VERSION)
  {
    switch(version)
    {
      case 1:
        if(!migrateV1ToV2())  return false;
        version = 2;
      break;
      case 2:
        if(!migrateV2ToV3())  return false;
        version = 3;
      break;
      case 3:
        if(!migrateV3ToV4())  return false;
        version = 4;
      break;
      case 4:
        if(!migrateV4ToV5())  return false;
        version = 5;
      break;
      case 5:
        if(!migrateV5ToV6())  return false;
        version = 6;
      break;
      default:
        qCritical() << "Unknown schema version:" << version;
      return false;
    }
  }

  return true;
}

bool Database::migrateV1ToV2()
{
  qDebug() << "Migrating schema v1 => v2";

  db_.transaction();

  QSqlQuery q(db_);

  const QStringList stmts = {
      "ALTER TABLE projects ADD COLUMN width INTEGER NOT NULL DEFAULT 1920",
      "ALTER TABLE projects ADD COLUMN height INTEGER NOT NULL DEFAULT 1080",
      "ALTER TABLE projects ADD COLUMN framerate REAL NOT NULL DEFAULT 30.0"
  };

  for(const QString& stmt : stmts)
  {
    if(!q.exec(stmt))
    {
      qCritical() << "Migration v1=>v2 failed:" << q.lastError();
      db_.rollback();
      return false;
    }
  }

  if(!setSchemaVersion(2))
  {
    db_.rollback();
    return false;
  }

  return db_.commit();
}

bool Database::migrateV2ToV3()
{
  qDebug() << "Migrating schema v2 => v3";

  db_.transaction();

  QSqlQuery q(db_);

  // 1 Create table
  if(!q.exec(R"(
        CREATE TABLE IF NOT EXISTS transitions (
            id TEXT PRIMARY KEY,
            type INTEGER NOT NULL,
            name TEXT NOT NULL,
            ms_duration INTEGER NOT NULL
        )
    )")) {
    db_.rollback();
    qWarning() << "Failed to create transitions table:" << q.lastError();
    return false;
  }

  // 2 Insert default transitions
  auto insertTransition = [&](TransitionType type,
    const QString& name,
    int duration)
    {
      QSqlQuery iq(db_);
      iq.prepare(R"(
            INSERT INTO transitions (id, type, name, ms_duration)
            VALUES (?, ?, ?, ?)
        )");

      iq.addBindValue(QUuid::createUuid().toString(QUuid::WithoutBraces));
      iq.addBindValue(static_cast<int>(type));
      iq.addBindValue(name);
      iq.addBindValue(duration);

      return iq.exec();
    };

  if(!insertTransition(TransitionType::TT_CUT, "Cut", 0) ||
    !insertTransition(TransitionType::TT_FADE, "Fade", 200) ||
    !insertTransition(TransitionType::TT_SLIDE, "Slide", 300))
  {
    db_.rollback();
    qWarning() << "Failed inserting default transitions";
    return false;
  }

  if(!setSchemaVersion(3))
  {
    db_.rollback();
    return false;
  }

  return db_.commit();
}

QStringList Database::schemaV1Statements() const
{
  return {

          R"(CREATE TABLE meta (
            key TEXT PRIMARY KEY,
            value TEXT
        ))",

        R"(INSERT INTO meta(key, value) VALUES ('schema_version', '1'))",

        R"(CREATE TABLE projects (
            id TEXT PRIMARY KEY,
            name TEXT NOT NULL,
            description TEXT,
            created_at TEXT NOT NULL,
            modified_at TEXT NOT NULL
        ))",

        R"(CREATE TABLE scenes (
            id TEXT PRIMARY KEY,
            project_id TEXT NOT NULL,
            name TEXT NOT NULL,
            order_index INTEGER NOT NULL,
            settings TEXT,
            FOREIGN KEY(project_id) REFERENCES projects(id) ON DELETE CASCADE
        ))",

        R"(CREATE TABLE sources (
            id TEXT PRIMARY KEY,
            scene_id TEXT NOT NULL,
            type INTEGER NOT NULL,
            name TEXT NOT NULL,
            order_index INTEGER NOT NULL,
            config TEXT,
            FOREIGN KEY(scene_id) REFERENCES scenes(id) ON DELETE CASCADE
        ))"
  };
}

bool Database::execSchemaStatements(const QStringList& statements)
{
  db_.transaction();

  QSqlQuery q;
  for(const QString& stmt : statements)
  {
    if(!q.exec(stmt))
    {
      qCritical() << "Schema error:" << q.lastError();
      db_.rollback();
      return false;
    }
  }

  return db_.commit();
}

bool Database::saveProject(const Project& project)
{
  QMutexLocker lock(&mutex_);

  if(!db_.isOpen()) return false;

  db_.transaction();

  QSqlQuery q;
  q.prepare(R"(
        INSERT INTO projects(id, name, description, width, height, framerate, created_at, modified_at)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)
        ON CONFLICT(id) DO UPDATE SET
            name=excluded.name,
            description=excluded.description,
            width=excluded.width,
            height=excluded.height,
            framerate=excluded.framerate,
            modified_at=excluded.modified_at
    )");

  q.addBindValue(project.id.toString(QUuid::WithoutBraces));
  q.addBindValue(project.name);
  q.addBindValue(project.description);
  q.addBindValue(project.width);
  q.addBindValue(project.height);
  q.addBindValue(project.framerate);
  q.addBindValue(project.createdAt.toUTC().toString(Qt::ISODate));
  q.addBindValue(project.modifiedAt.toUTC().toString(Qt::ISODate));

  if(!q.exec()) goto fail;

  for(const Scene& scene : project.scenes)
  {

    QSqlQuery qs;
    qs.prepare(R"(
            INSERT INTO scenes(id, project_id, name, order_index, settings, color_index)
            VALUES (?, ?, ?, ?, ?, ?)
            ON CONFLICT(id) DO UPDATE SET
                name=excluded.name,
                order_index=excluded.order_index,
                settings=excluded.settings,
                color_index=excluded.color_index
        )");

    qs.addBindValue(scene.id.toString(QUuid::WithoutBraces));
    qs.addBindValue(project.id.toString(QUuid::WithoutBraces));
    qs.addBindValue(scene.name);
    qs.addBindValue(scene.orderIndex);
    qs.addBindValue(QJsonDocument(scene.settings).toJson(QJsonDocument::Compact));
    qs.addBindValue(scene.colorIndex);

    if(!qs.exec()) goto fail;

    for(const Source& src : scene.sources) {

      QSqlQuery qsrc;
      qsrc.prepare(R"(
                INSERT INTO sources(id, scene_id, type, name, order_index, config, original_id)
                VALUES (?, ?, ?, ?, ?, ?, ?)
                ON CONFLICT(id) DO UPDATE SET
                    type=excluded.type,
                    name=excluded.name,
                    order_index=excluded.order_index,
                    config=excluded.config,
                    original_id=excluded.original_id
            )");

      qsrc.addBindValue(src.id.toString(QUuid::WithoutBraces));
      qsrc.addBindValue(scene.id.toString(QUuid::WithoutBraces));
      qsrc.addBindValue((int) src.type);
      qsrc.addBindValue(src.name);
      qsrc.addBindValue(src.orderIndex);
      qsrc.addBindValue(QJsonDocument(src.config).toJson(QJsonDocument::Compact));
      qsrc.addBindValue(src.originalId.toString(QUuid::WithoutBraces));

      if(!qsrc.exec()) goto fail;
    }
  }

  return db_.commit();

fail:
  qCritical() << "Save failed:" << q.lastError();
  db_.rollback();
  return false;
}

std::optional<Project> Database::loadProject(const QUuid& projectId)
{
  QMutexLocker lock(&mutex_);

  QSqlQuery q;
  q.prepare("SELECT * FROM projects WHERE id=?");
  q.addBindValue(projectId.toString(QUuid::WithoutBraces));

  if(!q.exec() || !q.next()) return std::nullopt;

  Project p;
  p.id = projectId;
  p.name = q.value("name").toString();
  p.description = q.value("description").toString();
  p.width = q.value("width").toInt();
  p.height = q.value("height").toInt();
  p.framerate = q.value("framerate").toDouble();
  p.createdAt = QDateTime::fromString(q.value("created_at").toString(), Qt::ISODate);
  p.modifiedAt = QDateTime::fromString(q.value("modified_at").toString(), Qt::ISODate);

  QSqlQuery qs;
  qs.prepare("SELECT * FROM scenes WHERE project_id=? ORDER BY order_index");
  qs.addBindValue(projectId.toString(QUuid::WithoutBraces));
  qs.exec();

  while(qs.next())
  {
    Scene s;
    s.id = QUuid(qs.value("id").toString());
    s.projectId = projectId;
    s.name = qs.value("name").toString();
    s.orderIndex = qs.value("order_index").toInt();
    s.settings = QJsonDocument::fromJson(
      qs.value("settings").toByteArray()
    ).object();
    s.colorIndex = qs.value("color_index").toInt();

    QSqlQuery qsrc;
    qsrc.prepare("SELECT * FROM sources WHERE scene_id=? ORDER BY order_index");
    qsrc.addBindValue(s.id.toString(QUuid::WithoutBraces));
    qsrc.exec();

    while(qsrc.next())
    {
      Source src;
      src.id = QUuid(qsrc.value("id").toString());
      src.sceneId = s.id;
      src.type = (SourceType) qsrc.value("type").toInt();
      src.name = qsrc.value("name").toString();
      src.orderIndex = qsrc.value("order_index").toInt();
      src.config = QJsonDocument::fromJson(qsrc.value("config").toByteArray()).object();
      src.originalId = QUuid(qsrc.value("original_id").toString());

      s.sources.push_back(src);
    }

    p.scenes.push_back(s);

    // sort by order index
    std::sort(p.scenes.begin(), p.scenes.end(), [](const Scene& a, const Scene& b) { return a.orderIndex < b.orderIndex; });
  }

  return p;
}

bool Database::deleteProject(const QUuid& projectId)
{
  QMutexLocker lock(&mutex_);

  if(!db_.isOpen()) return false;

  db_.transaction();

  QSqlQuery q;
  q.prepare("DELETE FROM projects WHERE id=?");
  q.addBindValue(projectId.toString(QUuid::WithoutBraces));

  if(!q.exec())
  {
    qCritical() << "Delete project failed:" << q.lastError();
    db_.rollback();
    return false;
  }

  // Optional: check if something was actually deleted
  if(q.numRowsAffected() == 0)
  {
    qWarning() << "No project deleted (id not found)";
    db_.rollback();
    return false;
  }

  return db_.commit();
}

int Database::schemaVersion() const
{
  if(!db_.isOpen())
    return -1;

  QSqlQuery q(db_);
  q.prepare("SELECT value FROM meta WHERE key='schema_version'");

  if(!q.exec())
  {
    qCritical() << "Failed to read schema_version:" << q.lastError();
    return -1;
  }

  if(!q.next()) return 0; // meta exists but no version yet

  return q.value(0).toInt();
}

bool Database::setSchemaVersion(int version)
{
  if(!db_.isOpen()) return false;

  QSqlQuery q(db_);
  q.prepare(R"(
        INSERT INTO meta(key, value)
        VALUES ('schema_version', ?)
        ON CONFLICT(key) DO UPDATE SET
            value=excluded.value
    )");

  q.addBindValue(QString::number(version));

  if(!q.exec())
  {
    qCritical() << "Failed to set schema_version:" << q.lastError();
    return false;
  }

  return true;
}

QVector<Project> Database::listProjects()
{
  QVector<Project> projects;

  // QSqlQuery q("SELECT * FROM projects ORDER BY modified_at DESC");
  QSqlQuery q("SELECT * FROM projects ORDER BY created_at ASC");

  while(q.next())
  {
    Project p;
    p.id = QUuid(q.value("id").toString());
    p.name = q.value("name").toString();
    p.description = q.value("description").toString();
    p.createdAt = QDateTime::fromString(q.value("created_at").toString(), Qt::ISODate);
    p.modifiedAt = QDateTime::fromString(q.value("modified_at").toString(), Qt::ISODate);

    projects.push_back(p);
  }

  return projects;
}

bool Database::deleteScene(const QUuid& sceneId)
{
  QSqlQuery q;
  q.prepare("DELETE FROM scenes WHERE id = ?");
  q.addBindValue(sceneId.toString(QUuid::WithoutBraces));

  if(!q.exec())
  {
    qWarning() << "Delete scene failed:" << q.lastError();
    return false;
  }
  return true;
}

bool Database::deleteSource(const QUuid& sourceId)
{
  QSqlQuery q;
  q.prepare("DELETE FROM sources WHERE id = ?");
  q.addBindValue(sourceId.toString(QUuid::WithoutBraces));

  if(!q.exec())
  {
    qWarning() << "Delete source failed:" << q.lastError();
    return false;
  }
  return true;
}

QVector<Transition> Database::listTransitions()
{
  QVector<Transition> transitions;

  QSqlQuery q("SELECT * FROM transitions");

  while(q.next())
  {
    Transition t;
    t.id = QUuid(q.value("id").toString());
    t.name = q.value("name").toString();
    t.type = static_cast<TransitionType>(q.value("type").toInt());
    t.msDuration = q.value("ms_duration").toInt();

    transitions.push_back(t);
  }

  return transitions;
}

bool Database::migrateV3ToV4()
{
  qDebug() << "Migrating schema v3 => v4";

  db_.transaction();

  QSqlQuery q(db_);

  // 1 Create table
  if(!q.exec(R"(
        CREATE TABLE IF NOT EXISTS servers (
            id TEXT PRIMARY KEY,
            platform TEXT NOT NULL,
            name TEXT NOT NULL,
            url TEXT,
            key TEXT,
            order_index INTEGER NOT NULL,
            is_active INTEGER NOT NULL DEFAULT 0
        )
    )")) {
    db_.rollback();
    qWarning() << "Failed to create servers table:" << q.lastError();
    return false;
  }

  if(!setSchemaVersion(4))
  {
    db_.rollback();
    return false;
  }

  return db_.commit();
}

bool Database::migrateV4ToV5()
{
  qDebug() << "Migrating schema v4 => v5";

  db_.transaction();

  QSqlQuery q(db_);

  if(!q.exec(R"(
        ALTER TABLE sources
        ADD COLUMN original_id TEXT
    )")) {
    db_.rollback();
    qWarning() << "Failed to add original_id column:" << q.lastError();
    return false;
  }

  if(!setSchemaVersion(5))
  {
    db_.rollback();
    return false;
  }

  return db_.commit();
}

bool Database::migrateV5ToV6()
{
  qDebug() << "Migrating schema v5 => v6";

  db_.transaction();

  QSqlQuery q(db_);

  if(!q.exec(R"(
        ALTER TABLE scenes
        ADD COLUMN color_index INTEGER
    )")) {
    db_.rollback();
    qWarning() << "Failed to add color_index column:" << q.lastError();
    return false;
  }

  if(!setSchemaVersion(6))
  {
    db_.rollback();
    return false;
  }

  return db_.commit();
}

QVector<StreamingServer> Database::listStreamingServers()
{
  QVector<StreamingServer> streamingServers;

  QSqlQuery q("SELECT * FROM servers");

  while(q.next())
  {
    StreamingServer ss;
    ss.id = QUuid(q.value("id").toString());
    ss.platform = q.value("platform").toString();
    ss.name = q.value("name").toString();
    ss.url = q.value("url").toString();
    ss.key = q.value("key").toString();
    ss.enabled = q.value("is_active").toInt() == 1;
    ss.orderIndex = q.value("order_index").toInt();

    streamingServers.push_back(ss);
  }

  std::sort(streamingServers.begin(), streamingServers.end(), [](const StreamingServer& a, const StreamingServer& b) { return a.orderIndex < b.orderIndex; });

  return streamingServers;
}

bool Database::saveStreamingServer(const StreamingServer& _streamingServer)
{
  QMutexLocker lock(&mutex_);

  if(!db_.isOpen()) return false;

  db_.transaction();

  QSqlQuery q;
  q.prepare(R"(
        INSERT INTO servers(id, platform, name, url, key, is_active, order_index)
        VALUES (?, ?, ?, ?, ?, ?, ?)
        ON CONFLICT(id) DO UPDATE SET
            platform=excluded.platform,
            name=excluded.name,
            url=excluded.url,
            key=excluded.key,
            is_active=excluded.is_active,
            order_index=excluded.order_index
    )");

  q.addBindValue(_streamingServer.id.toString(QUuid::WithoutBraces));
  q.addBindValue(_streamingServer.platform);
  q.addBindValue(_streamingServer.name);
  q.addBindValue(_streamingServer.url);
  q.addBindValue(_streamingServer.key);
  q.addBindValue(_streamingServer.enabled? 1 : 0);
  q.addBindValue(_streamingServer.orderIndex);

  if(!q.exec()) goto fail;

  return db_.commit();

fail:
  QString aux = q.lastError().text();
  qCritical() << "Save failed:" << q.lastError();
  db_.rollback();
  return false;
}

bool Database::removeStreamingServer(const QUuid& _id)
{
  QMutexLocker lock(&mutex_);

  if(!db_.isOpen()) return false;

  db_.transaction();

  QSqlQuery q;
  q.prepare("DELETE FROM servers WHERE id=?");
  q.addBindValue(_id.toString(QUuid::WithoutBraces));

  if(!q.exec())
  {
    qCritical() << "Delete server failed:" << q.lastError();
    db_.rollback();
    return false;
  }

  // Optional: check if something was actually deleted
  if(q.numRowsAffected() == 0)
  {
    qWarning() << "No server deleted (id not found)";
    db_.rollback();
    return false;
  }

  return db_.commit();
}

bool Database::removeStreamingServers()
{
  QMutexLocker lock(&mutex_);

  if(!db_.isOpen()) return false;

  db_.transaction();

  QSqlQuery q;
  q.prepare("DELETE FROM servers");

  if(!q.exec())
  {
    qCritical() << "Delete servers failed:" << q.lastError();
    db_.rollback();
    return false;
  }

  return db_.commit();
}