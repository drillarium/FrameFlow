#include "livesourcewidget.h"
#include "sourcereaderbaserenderer.h"

LiveSourceWidget::LiveSourceWidget(QWidget *parent)
:BaseSourceWidget(parent)
{
  ui.setupUi(this);
}

LiveSourceWidget::~LiveSourceWidget()
{

}

void LiveSourceWidget::init()
{
  fillLiveCombo();
}

void LiveSourceWidget::fillLiveCombo()
{
  ui.deviceLineComboBox->clear();

  QStringList sl = SourceReaderBaseRenderer::listOfLiveSources();
  ui.deviceLineComboBox->addItems(sl);
}

Source LiveSourceWidget::source()
{
  Source s = Source();
  Source& source = editing_ ? source_ : s;

  QJsonObject jsonConfig = editing_ ? source_.config : QJsonObject();
  jsonConfig.insert("line", ui.deviceLineComboBox->currentText());
  if(!editing_)
  {
    jsonConfig["x"] = 10;
    jsonConfig["y"] = 10;
    jsonConfig["width"] = 320;
    jsonConfig["height"] = 240;
  }

  // Source source;
  source.name = ui.nameLineEdit->text();
  source.type = type();
  source.config = jsonConfig;

  return source;
}

bool LiveSourceWidget::isValid()
{
  return (ui.nameLineEdit->text().size() > 0);
}

void LiveSourceWidget::editSource(const Source& _source)
{
  BaseSourceWidget::editSource(_source);

  ui.nameLineEdit->setText(source_.name);
  if(source_.config.contains("line") && source_.config.value("line").isString())
  {
    QString device = source_.config["line"].toString();
    ui.deviceLineComboBox->setCurrentText(device);
  }
}
