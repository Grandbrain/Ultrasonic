#include "window.h"
#include "ui_window.h"
#include "about.h"

Window::Window(QWidget* parent) : QMainWindow(parent), ui(new Ui::Window)
{
    ui->setupUi(this);
    ui->widgetPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes);
    ui->widgetPlot->axisRect()->setupFullAxesBox();
    ui->widgetPlot->xAxis->setLabel(tr("Время"));
    ui->widgetPlot->yAxis->setLabel(tr("Расстояние"));

    connect(ui->actionAbout, SIGNAL(triggered(bool)), SLOT(OnAbout()));
}

Window::~Window()
{
    delete ui;
}


void Window::OnAbout()
{
    About about(this);
    about.exec();
}