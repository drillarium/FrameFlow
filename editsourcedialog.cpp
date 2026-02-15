#include "editsourcedialog.h"
#include <QTimer>

EditSourceDialog::EditSourceDialog(const Source &_source, QWidget *parent)
:QDialog(parent)
,source_(_source)
{
  ui.setupUi(this);

  setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
  setAttribute(Qt::WA_TranslucentBackground);

  ui.customStackedWidget->setCurrentIndex((int) _source.type);
  BaseSourceWidget* w = static_cast<BaseSourceWidget*>(ui.customStackedWidget->widget((int) source_.type));
  w->init();
  w->editSource(_source);
  ui.customStackedWidget->setFixedHeight(w->h());

  setFixedWidth(width());
  setFixedHeight(ui.customStackedWidget->height() + 150);
}

EditSourceDialog::~EditSourceDialog()
{

}

void EditSourceDialog::onAccept()
{
  BaseSourceWidget* w = static_cast<BaseSourceWidget*>(ui.customStackedWidget->widget((int) source_.type));
  if(w->isValid())
  {
    source_ = w->source();
    accept();
  }
  else
  {
    ui.errorLabel->setText("ERROR: Check parameters");
    QTimer::singleShot(5000, [&]() { ui.errorLabel->clear(); });
  }
}
