#include "selectsourcedialog.h"
#include "sourceitemwidget.h"
#include "source_model.h"
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QTimer>
#include "project_manager.h"

class SourceItemDelegate : public QStyledItemDelegate
{
public:
  using QStyledItemDelegate::QStyledItemDelegate;

  void paint(QPainter* painter,
    const QStyleOptionViewItem& option,
    const QModelIndex& index) const override
  {
    painter->save();

    // selection background
    if(option.state & QStyle::State_Selected)
      painter->fillRect(option.rect, option.palette.highlight());

    // icon
    QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
    QRect iconRect = option.rect.adjusted(8, 8, -8, -8);
    iconRect.setWidth(40);
    icon.paint(painter, iconRect);

    // text rects
    QRect textRect = option.rect.adjusted(56, 8, -8, -8);

    QString title = index.data(Qt::DisplayRole).toString();
    QString desc = index.data(Qt::UserRole + 1).toString();

    // title
    QFont titleFont = option.font;
    titleFont.setBold(true);
    painter->setFont(titleFont);
    painter->setPen(Qt::white);
    painter->drawText(textRect, Qt::AlignTop | Qt::AlignLeft, title);

    // description
    QFont descFont = option.font;
    descFont.setPointSize(descFont.pointSize() - 1);
    painter->setFont(descFont);
    painter->setPen(option.palette.color(QPalette::Disabled, QPalette::Text));

    QRect descRect = textRect.adjusted(0, 20, 0, 0);
    painter->drawText(descRect, Qt::AlignTop | Qt::AlignLeft, desc);

    painter->restore();
  }

  QSize sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const override
  {
    return QSize(200, 56);
  }
};

bool isTypeEnabled(SourceType type)
{
  switch(type)
  {
    case SourceType::EST_COLOR:
    case SourceType::EST_FILE:
    case SourceType::EST_URL:
    case SourceType::EST_LIVE_SOURCE:
    case SourceType::EST_NDI:
    case SourceType::EST_DEVICE:
    case SourceType::EST_SCREEN_CAPTURE:
    case SourceType::EST_TEXT: return true;
  }

  return false;
}

SelectSourceDialog::SelectSourceDialog(QWidget *parent)
:QDialog(parent)
{
  ui.setupUi(this);

  setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
  setAttribute(Qt::WA_TranslucentBackground);

  /* sources */  
  for(int i = 0; i < (int) SourceType::EST_LAST; i++)
  {
    QListWidgetItem* lwi = new QListWidgetItem(ui.listWidget);
    lwi->setSizeHint(QSize(150, 75));
    SourceItemWidget* siw = new SourceItemWidget((SourceType) i);
    siw->setEnabled(isTypeEnabled((SourceType) i));
    ui.listWidget->addItem(lwi);
    ui.listWidget->setItemWidget(lwi, siw);
  }
  ui.listWidget->item(0)->setSelected(true);

  connect(ui.buttonGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), this, [this](QAbstractButton* button) {
    if(button == ui.newSourceButton) {
      ui.pageStackedWidget->setCurrentIndex(0);
      onItemSelectedChange();
    }
    else if(button == ui.copySceneButton) {
      
      ui.pageStackedWidget->setCurrentIndex(1);
      onItemSelectedChange();
    }
  });

  QStandardItemModel* model = new QStandardItemModel(this);
  ui.listView->setModel(model);
  ui.listView->setItemDelegate(new SourceItemDelegate(ui.listView));

  // iterate over all sources not cloned from another one
  ProjectManager &pm = ProjectManager::instance();
  auto project = pm.currentProject();
  if(project)
  {
    for(Scene scene : project->scenes)
    {
      for(Source source : scene.sources)
      {
        if(source.originalId.isNull())
        {
          QStandardItem* item = new QStandardItem();
          item->setText(source.name);
          item->setData(QIcon(":/FrameFlow/tv.svg"), Qt::DecorationRole);
          item->setData(sourceTitle(source.type), Qt::UserRole + 1);
          item->setData(source.id, Qt::UserRole + 2);
          model->appendRow(item);
        }
      }
    }
  }
}

SelectSourceDialog::~SelectSourceDialog()
{

}

void SelectSourceDialog::onAccept()
{
  int index = ui.pageStackedWidget->currentIndex();
  if(index == 0)
  {
    for(int i = 0; i < ui.listWidget->count(); ++i)
    {
      QListWidgetItem* item = ui.listWidget->item(i);
      if(item->isSelected())
      {
        if(i < ui.customStackedWidget->count())
        {
          BaseSourceWidget* w = static_cast<BaseSourceWidget*>(ui.customStackedWidget->widget(i));
          if(w->isValid())
          {
            source_ = w->source();
            accept();
          }
          else
          {
            ui.errorLabel->setText("ERROR: Check parameters");
            QTimer::singleShot(5000, [&](){ ui.errorLabel->clear(); });
          }
          return;
        }
      }
    }
    reject();
  }
  else
  {
    QModelIndex index = ui.listView->currentIndex();
    if(index.isValid())
    {
      QVariant value = index.data(Qt::UserRole + 2);
      QUuid sourceId = value.toUuid();
      
      ProjectManager& pm = ProjectManager::instance();
      auto project = pm.currentProject();
      if(project)
      {
        for(Scene scene : project->scenes)
        {
          for(Source source : scene.sources)
          {
            if(source.id == sourceId)
            {
              source_ = source;
              source_.originalId = sourceId;
              source_.name = source_.name + "_copy";
              accept();
              return;
            }
          }
        }
      }
    }
    reject();
  }
}

void SelectSourceDialog::onItemSelectedChange()
{
  int index = ui.pageStackedWidget->currentIndex();
  if(index == 0)
  {
    int selected = -1;
    for(int i = 0; i < ui.listWidget->count(); ++i)
    {
      QListWidgetItem* item = ui.listWidget->item(i);
      SourceItemWidget* w = static_cast<SourceItemWidget*>(ui.listWidget->itemWidget(item));
      if(w) w->setSelected(item->isSelected() && isTypeEnabled((SourceType) i));
      if(item->isSelected()) selected = i;
    }

    if(!isTypeEnabled((SourceType) selected)) return;

    if(selected >= 0 && selected < ui.customStackedWidget->count())
    {
      ui.customStackedWidget->setCurrentIndex(selected);
      BaseSourceWidget *w = static_cast<BaseSourceWidget*>(ui.customStackedWidget->widget(selected));
      w->init();
      ui.customStackedWidget->setFixedHeight(w->h());
    }
    else
    {
      ui.customStackedWidget->setCurrentIndex(0);
      ui.customStackedWidget->setFixedHeight(0);
    }
    setFixedWidth(width());
    ui.customStackedWidget->show();
    setFixedHeight(ui.listWidget->height() + ui.customStackedWidget->height() + 120);
  }
  else if(index == 1)
  {
    setFixedWidth(width());
    ui.customStackedWidget->hide();
    setFixedHeight(ui.listView->height() + 120);
  }
}
