#include "urlpickerwidget.h"

UrlPickerWidget::UrlPickerWidget(QWidget *parent)
:BaseSourceWidget(parent)
{
  ui.setupUi(this);
}

UrlPickerWidget::~UrlPickerWidget()
{
}

Source UrlPickerWidget::source()
{
  Source s = Source();
  Source& source = editing_ ? source_ : s;

  QJsonObject jsonConfig;
  jsonConfig.insert("url", ui.pathLineEdit->text());

  source.name = ui.nameLineEdit->text();
  source.type = type();
  source.config = jsonConfig;

  return source;
}

bool UrlPickerWidget::isValid()
{
  return (ui.nameLineEdit->text().size() > 0) && (ui.pathLineEdit->text().size() > 0);
}

void UrlPickerWidget::editSource(const Source& _source)
{
  BaseSourceWidget::editSource(_source);

  ui.nameLineEdit->setText(source_.name);

  if(source_.config.contains("path") && source_.config.value("path").isString())
  {
    QString path = source_.config.value("path").toString();
    ui.pathLineEdit->setText(path);
  }
}
