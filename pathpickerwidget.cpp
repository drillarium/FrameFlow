#include "pathpickerwidget.h"
#include <QFileDialog>

PathPickerWidget::PathPickerWidget(QWidget *parent)
:BaseSourceWidget(parent)
{
  ui.setupUi(this);
}

PathPickerWidget::~PathPickerWidget()
{
}

Source PathPickerWidget::source()
{
  Source s = Source();
  Source& source = editing_ ? source_ : s;

  QJsonObject jsonConfig;
  jsonConfig.insert("path", ui.pathLineEdit->text());
  jsonConfig.insert("loop", ui.loopCheckBox->text());

  source.name = ui.nameLineEdit->text();
  source.type = type();
  source.config = jsonConfig;

  return source;
}

void PathPickerWidget::onPickPath()
{
  QString initPath = ui.pathLineEdit->text();
  if(initPath.isEmpty()) initPath = QDir::homePath();
  QString file = QFileDialog::getOpenFileName(this, "Select file", initPath, "All files (*)");

  if(!file.isEmpty())
  {
    ui.pathLineEdit->setText(file);
  }
}

bool PathPickerWidget::isValid()
{
  return (ui.nameLineEdit->text().size() > 0) && (ui.pathLineEdit->text().size() > 0);
}

void PathPickerWidget::editSource(const Source& _source)
{
  BaseSourceWidget::editSource(_source);

  ui.nameLineEdit->setText(source_.name);
  
  if(source_.config.contains("path") && source_.config.value("path").isString())
  {
    QString path = source_.config.value("path").toString();
    ui.pathLineEdit->setText(path);
  }

  if(source_.config.contains("loop") && source_.config.value("loop").isBool())
  {
    bool loop = source_.config.value("loop").toBool();
    ui.loopCheckBox->setChecked(loop);
  }
}
