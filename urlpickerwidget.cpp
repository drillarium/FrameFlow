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

  QJsonObject jsonConfig = editing_ ? source_.config : QJsonObject();
  jsonConfig.insert("url", ui.pathLineEdit->text());
  if(!editing_)
  {
    jsonConfig["x"] = 400;
    jsonConfig["y"] = 400;
    jsonConfig["width"] = 320;
    jsonConfig["height"] = 240;
  }

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

  if(source_.config.contains("url") && source_.config.value("url").isString())
  {
    QString path = source_.config.value("url").toString();
    ui.pathLineEdit->setText(path);
  }
}
