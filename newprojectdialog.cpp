#include "newprojectdialog.h"

NewProjectDialog::NewProjectDialog(QWidget *parent)
:QDialog(parent)
{
  ui.setupUi(this);

  setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
  setAttribute(Qt::WA_TranslucentBackground);
  onSelectionChange();
}

NewProjectDialog::~NewProjectDialog()
{
}

void NewProjectDialog::onCreateProject()
{
  project_.name = ui.lineEdit->text();
  accept();
}

void NewProjectDialog::onSelectionChange()
{
  int vrIndex = ui.vrComboBox->currentIndex();
  if(vrIndex == 0) /* Full HD */
  {
    ui.aspectRatioLabel->setText("Aspect ratio: 16:9");
    ui.resolutionLabel->setText("Resolution: 1920 x 1080");
    project_.width = 1920;
    project_.height = 1080;    
  }
  else if(vrIndex == 1) /* 2K */
  {
    ui.aspectRatioLabel->setText("Aspect ratio: 16:9");
    ui.resolutionLabel->setText("Resolution: 2560 x 1440");
    project_.width = 2560;
    project_.height = 1440;
  }
  else if(vrIndex == 2) /* 4K */
  {
    ui.aspectRatioLabel->setText("Aspect ratio: 16:9");
    ui.resolutionLabel->setText("Resolution: 3840 x 2160");
    project_.width = 3840;
    project_.height = 2160;
  }
  else if(vrIndex == 3) /* HD */
  {
    ui.aspectRatioLabel->setText("Aspect ratio: 16:9");
    ui.resolutionLabel->setText("Resolution: 1280 x 720");
    project_.width = 1280;
    project_.height = 720;
  }
  else if(vrIndex == 4) /* Vertical HD */
  {
    ui.aspectRatioLabel->setText("Aspect ratio: 9:16");
    ui.resolutionLabel->setText("Resolution: 1080 x 1920");
    project_.width = 1080;
    project_.height = 1920;
  }
  else if(vrIndex == 5) /* Square */
  {
    ui.aspectRatioLabel->setText("Aspect ratio: 1:1");
    ui.resolutionLabel->setText("Resolution: 1080 x 1080");
    project_.width = 1080;
    project_.height = 1080;
  }
  int frIndex = ui.frComboBox->currentIndex();
  if(frIndex == 0) /* Cinema */
  {
    ui.framerateLabel->setText("Frame Rate: 24 fps");
    project_.framerate = 24;
  }
  else if(frIndex == 1) /* PAL */
  {
    ui.framerateLabel->setText("Frame Rate: 25 fps");
    project_.framerate = 25;
  }
  else if(frIndex == 2) /* Standard */
  {
    ui.framerateLabel->setText("Frame Rate: 30 fps");
    project_.framerate = 30;
  }
  else if(frIndex == 3) /* PAL High */
  {
    ui.framerateLabel->setText("Frame Rate: 50 fps");
    project_.framerate = 50;
  }
  else if(frIndex == 4) /* Smooth */
  {
    ui.framerateLabel->setText("Frame Rate: 60 fps");
    project_.framerate = 60;
  }
  else if(frIndex == 5) /* High Speed */
  {
    ui.framerateLabel->setText("Frame Rate: 120 fps");
    project_.framerate = 120;
  }
}
