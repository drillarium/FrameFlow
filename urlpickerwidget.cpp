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
  QJsonObject jsonConfig;
  jsonConfig.insert("url", ui.pathLineEdit->text());

  Source source;
  source.name = ui.nameLineEdit->text();
  source.type = type();
  source.config = jsonConfig;

  return source;
}

bool UrlPickerWidget::isValid()
{
  return (ui.nameLineEdit->text().size() > 0) && (ui.pathLineEdit->text().size() > 0);
}