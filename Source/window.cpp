#include "about.h"
#include "window.h"
#include "ui_window.h"

int resTimeShow = 8000;
QString resIconSuccess = ":/root/Resources/success.ico";
QString resIconFail = ":/root/Resources/error.ico";
QString resIconRun = ":/root/Resources/run.ico";
QString resIconPause = ":/root/Resources/pause.ico";
QString resStyleSuccess = ".QWidget { background-color: rgb(198, 255, 202); border: 1px solid green; }";
QString resStyleFail = ".QWidget { background-color: rgb(255, 198, 202); border: 1px solid red; }";
QString resTextSuccess = QObject::tr("Подключение успешно выполнено");
QString resTextFail = QObject::tr("Не удалось выполнить подключение");
QString resTextRun = QObject::tr("Продолжить");
QString resTextPause = QObject::tr("Пауза");
QString resTextX = QObject::tr("Время");
QString resTextY = QObject::tr("Выходной сигнал");
QString resTextFormat = "hh:mm:ss";

Window::Window(QWidget* parent) : QMainWindow(parent), ui(new Ui::Window)
{
    ui->setupUi(this);
    ui->widgetPlot->xAxis->setLabel(resTextX);
    ui->widgetPlot->yAxis->setLabel(resTextY);
    ui->widgetPlot->addGraph();
    ui->widgetPlot->graph(0)->setPen(QPen(Qt::blue));
    ui->widgetPlot->graph(0)->setBrush(QBrush(QColor(240, 255, 200)));
    ui->widgetPlot->addGraph();
    ui->widgetPlot->graph(1)->setPen(QPen(Qt::blue));
    ui->widgetPlot->graph(1)->setLineStyle(QCPGraph::lsNone);
    ui->widgetPlot->graph(1)->setScatterStyle(QCPScatterStyle::ssDisc);
    ui->widgetPlot->graph(1)->removeFromLegend();
    ui->widgetPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    ui->widgetPlot->xAxis->setDateTimeFormat(resTextFormat);
    ui->widgetPlot->xAxis->setSelectableParts(QCPAxis::spAxisLabel);
    ui->widgetPlot->yAxis->setSelectableParts(QCPAxis::spAxisLabel);
    ui->widgetPlot->legend->setVisible(true);
    ui->widgetPlot->legend->setSelectableParts(QCPLegend::spItems);
    ui->buttonDisconnect->setEnabled(false);
    ui->comboPorts->addItems(ultrasonic.GetAvailablePorts());
    ui->buttonConnect->setEnabled(ui->comboPorts->count());
    ui->widgetConnectionMessage->installEventFilter(this);
    ui->actionStartStop->setEnabled(false);
    ui->tabWidget->setTabEnabled(2, false);
    ui->tabWidget->setTabEnabled(3, false);

    connect(ui->widgetPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->widgetPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->widgetPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->widgetPlot->yAxis2, SLOT(setRange(QCPRange)));
    connect(ui->widgetPlot, SIGNAL(mouseWheel(QWheelEvent*)), SLOT(OnMouseWheel()));
    connect(ui->actionStartStop, SIGNAL(triggered(bool)), SLOT(OnStartStop()));
    connect(ui->actionAbout, SIGNAL(triggered(bool)), SLOT(OnAbout()));
    connect(ui->buttonConnect, SIGNAL(released()), SLOT(OnConnect()));
    connect(ui->buttonDisconnect, SIGNAL(released()), SLOT(OnDisconnect()));
    connect(ui->spinAutosave, SIGNAL(editingFinished()), SLOT(OnAutosaveChange()));
    connect(&renderTimer, SIGNAL(timeout()), SLOT(OnRender()));
    connect(&saveTimer, SIGNAL(timeout()), SLOT(OnAutosave()));
    connect(&ultrasonic, SIGNAL(OnData(QByteArray)), SLOT(OnData(QByteArray)));

    renderTimer.start();
    OnAutosaveChange();
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

void Window::OnConnect()
{
    ui->widgetConnectionMessage->hide();
    bool connected = ultrasonic.Connect(ui->comboPorts->currentText());
    if(connected)
    {
        ui->buttonConnect->setEnabled(false);
        ui->buttonDisconnect->setEnabled(true);
        ui->labelConnectionMessage->setText(resTextSuccess);
        ui->labelConnectionResult->setPixmap(resIconSuccess);
        ui->widgetConnectionMessage->setStyleSheet(resStyleSuccess);
        ui->widgetConnectionMessage->show();
        ui->actionStartStop->setEnabled(true);
        ui->tabWidget->setTabEnabled(2, true);
        ui->tabWidget->setTabEnabled(3, true);
        time.restart();
        QTimer::singleShot(resTimeShow, ui->widgetConnectionMessage, SLOT(hide()));
    }
    else
    {
        ui->labelConnectionMessage->setText(resTextFail);
        ui->labelConnectionResult->setPixmap(resIconFail);
        ui->widgetConnectionMessage->setStyleSheet(resStyleFail);
        ui->widgetConnectionMessage->show();
        QTimer::singleShot(resTimeShow, ui->widgetConnectionMessage, SLOT(hide()));
    }
}

void Window::OnDisconnect()
{
    ui->widgetConnectionMessage->hide();
    ui->buttonConnect->setEnabled(true);
    ui->buttonDisconnect->setEnabled(false);
    ui->actionStartStop->setEnabled(false);
    ui->tabWidget->setTabEnabled(2, false);
    ui->widgetPlot->graph(0)->clearData();
    ui->widgetPlot->graph(1)->clearData();
    ui->widgetPlot->replot();
    ui->widgetPlot->setInteractions(QCP::iSelectLegend);
    ultrasonic.Disconnect();
}

void Window::OnStartStop()
{
    bool stopped = ui->actionStartStop->text() == resTextRun;
    QString icon = stopped ? resIconPause : resIconRun;
    QString text = stopped ? resTextPause : resTextRun;
    QCP::Interactions interactions = stopped ? QCP::iSelectLegend : QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes | QCP::iSelectLegend;
    ui->actionStartStop->setIcon(QIcon(icon));
    ui->actionStartStop->setText(text);
    ui->widgetPlot->setInteractions(interactions);
}

void Window::OnAutosaveChange()
{
    saveTimer.start(ui->spinAutosave->value() * 60000);
}

void Window::OnAutosave()
{
    if(saveArray.isEmpty()) return;
}

void Window::OnRender()
{
    if(array.isEmpty()) return;
    //qreal current = QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000.0;
    qreal current = time.elapsed() / 1000.0;
    double value = array.last();
    ui->widgetPlot->graph(0)->addData(current, value);
    ui->widgetPlot->graph(1)->clearData();
    ui->widgetPlot->graph(1)->addData(current, value);
    ui->widgetPlot->graph(0)->rescaleValueAxis();
    array.clear();
    ui->widgetPlot->xAxis->setRange(current + 0.25, 8, Qt::AlignRight);
    ui->widgetPlot->replot();
}

void Window::OnAxisNameChange()
{

}

void Window::OnLegendNameChange()
{

}

void Window::OnMouseWheel()
{
    if(ui->widgetPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->widgetPlot->axisRect()->setRangeZoom(ui->widgetPlot->xAxis->orientation());
    else if(ui->widgetPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->widgetPlot->axisRect()->setRangeZoom(ui->widgetPlot->yAxis->orientation());
    else ui->widgetPlot->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
}

void Window::OnData(const QByteArray& data)
{
    if(ui->actionStartStop->text() == resTextRun) return;
    QList<QByteArray> list = data.split(' ');
    foreach (QByteArray ar, list)
    {
        QByteArray tri = ar.trimmed();
        if(tri == "") continue;
        double val = tri.toDouble();
        array.append(val);
    }
}

bool Window::eventFilter(QObject* object, QEvent* event)
{
    if(object == ui->widgetConnectionMessage ||
       object == ui->labelConnectionMessage  ||
       object == ui->labelConnectionResult)
    {
        if(event->type() == QEvent::MouseButtonRelease)
            ui->widgetConnectionMessage->hide();
    }
    return true;
}