#include "editsourcedialog.h"

EditSourceDialog::EditSourceDialog(const Source &_source, QWidget *parent)
:QDialog(parent)
,source_(_source)
{
  ui.setupUi(this);

  setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
  setAttribute(Qt::WA_TranslucentBackground);

  ui.customStackedWidget->setCurrentIndex((int) _source.type);
  BaseSourceWidget* w = static_cast<BaseSourceWidget*>(ui.customStackedWidget->widget((int)_source.type));
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
  // TODO

  accept();
}
