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
  QJsonObject jsonConfig;
  jsonConfig.insert("path", ui.pathLineEdit->text());
  jsonConfig.insert("loop", ui.loopCheckBox->text());

  Source source;
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
