#include "about.h"
#include "window.h"
#include "ui_window.h"

int resTimeShow = 8000;
QString resIconSuccess = ":/root/Resources/success.ico";
QString resIconFail = ":/root/Resources/error.ico";
QString resIconRun = ":/root/Resources/run.ico";
QString resIconPause = ":/root/Resources/pause.ico";
QString resIconDocument = ":/root/Resources/document.ico";
QString resIconOpen = ":/root/Resources/open.ico";
QString resIconClosed = ":/root/Resources/closed.ico";
QString resStyleSuccess = ".QWidget { background-color: rgb(198, 255, 202); border: 1px solid green; }";
QString resStyleFail = ".QWidget { background-color: rgb(255, 198, 202); border: 1px solid red; }";
QString resTextFormat = "hh:mm:ss";
QString resTextApplication = "Ultrasonic";
QString resTextGroup = "Application";
QString resTextPath = "path";

Window::Window(QWidget* parent) : QMainWindow(parent), ui(new Ui::Window)
{
    ui->setupUi(this);
    ui->widgetPlot->addGraph();
    ui->widgetPlot->addGraph();
    ui->widgetPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes);
    ui->widgetPlot->graph(0)->setPen(QPen(Qt::blue));
    ui->widgetPlot->graph(0)->setBrush(QBrush(QColor(240, 255, 200)));
    ui->widgetPlot->graph(0)->setLineStyle(QCPGraph::lsLine);
    ui->widgetPlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 4));
    ui->widgetPlot->graph(1)->setPen(QPen(Qt::blue));
    ui->widgetPlot->graph(1)->setLineStyle(QCPGraph::lsNone);
    ui->widgetPlot->graph(1)->setScatterStyle(QCPScatterStyle::ssDisc);
    ui->widgetPlot->xAxis->setLabel(tr("Время, с"));
    ui->widgetPlot->yAxis->setLabel(tr("Выходной сигнал"));
    ui->widgetPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    ui->widgetPlot->xAxis->setDateTimeSpec(Qt::UTC);
    ui->widgetPlot->xAxis->setDateTimeFormat(resTextFormat);
    ui->widgetPlot->xAxis->setSelectableParts(QCPAxis::spAxisLabel | QCPAxis::spAxis);
    ui->widgetPlot->yAxis->setSelectableParts(QCPAxis::spAxisLabel | QCPAxis::spAxis);
    ui->widgetConnectionMessage->installEventFilter(this);
    ui->comboPorts->addItems(ultrasonic.GetAvailablePorts());
    ui->actionStartStop->setEnabled(false);
    ui->buttonConnect->setEnabled(ui->comboPorts->count());
    ui->buttonDisconnect->setEnabled(false);

    connect(ui->widgetPlot, SIGNAL(axisDoubleClick(QCPAxis*, QCPAxis::SelectablePart, QMouseEvent*)), SLOT(OnAxisNameChange(QCPAxis*, QCPAxis::SelectablePart)));
    connect(ui->widgetPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->widgetPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->widgetPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->widgetPlot->yAxis2, SLOT(setRange(QCPRange)));
    connect(ui->widgetPlot, SIGNAL(mouseWheel(QWheelEvent*)), SLOT(OnMouseWheel()));
    connect(ui->actionStartStop, SIGNAL(triggered(bool)), SLOT(OnStartStop()));
    connect(ui->actionAbout, SIGNAL(triggered(bool)), SLOT(OnAbout()));
    connect(ui->buttonExplorer, SIGNAL(released()), SLOT(OnExplorer()));
    connect(ui->buttonConnect, SIGNAL(released()), SLOT(OnConnect()));
    connect(ui->buttonDisconnect, SIGNAL(released()), SLOT(OnDisconnect()));
    connect(ui->editPath, SIGNAL(editingFinished()), SLOT(OnPathChanged()));
    connect(ui->treeHistory, SIGNAL(itemExpanded(QTreeWidgetItem*)), SLOT(OnTreeExpanded(QTreeWidgetItem*)));
    connect(ui->treeHistory, SIGNAL(itemCollapsed(QTreeWidgetItem*)), SLOT(OnTreeCollapsed(QTreeWidgetItem*)));
    connect(ui->treeHistory, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), SLOT(OnTreeDoubleClicked(QTreeWidgetItem*)));
    connect(&autosave, SIGNAL(timeout()), SLOT(OnAutosave()));
    connect(&ultrasonic, SIGNAL(OnData(QByteArray)), SLOT(OnData(QByteArray)));

    if(!ReadSettings())
        QMessageBox::warning(this, tr("Предупреждение"), tr("Не удалось загрузить настройки программы."));
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

void Window::OnExplorer()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly);
    if(dialog.exec() == QFileDialog::Rejected) return;
    QStringList selected = dialog.selectedFiles();
    if(selected.isEmpty()) return;
    ui->editPath->setText(selected.first());
    if(!LoadHistory()) QMessageBox::warning(this, tr("Предупреждение"), tr("Не удалось создать папку журнала. Попробуйте задать её самостоятельно."));
}

void Window::OnConnect()
{
    ui->widgetConnectionMessage->hide();
    bool connected = ultrasonic.Connect(ui->comboPorts->currentText());
    if(connected)
    {
        bool run = ui->actionStartStop->text() == tr("Пауза");
        if(run) ui->widgetPlot->setInteractions(QCP::iSelectAxes);
        ui->widgetPlot->graph(0)->clearData();
        ui->widgetPlot->graph(1)->clearData();
        ui->textConsole->clear();
        ui->widgetPlot->xAxis->setRange(0, 5);
        ui->widgetPlot->replot();
        ui->tabWidget->setTabEnabled(1, false);
        ui->buttonConnect->setEnabled(false);
        ui->buttonDisconnect->setEnabled(true);
        ui->labelConnectionMessage->setText(tr("Подключение успешно выполнено"));
        ui->labelConnectionResult->setPixmap(resIconSuccess);
        ui->widgetConnectionMessage->setStyleSheet(resStyleSuccess);
        ui->widgetConnectionMessage->show();
        ui->actionStartStop->setEnabled(true);
        timer.start();
        time.restart();
        if(!OpenFile()) QMessageBox::warning(this, tr("Предупреждение"), tr("Не удалось создать файл журнала."));
        autosave.start(60000);
        connect(&timer, SIGNAL(timeout()), SLOT(OnRender()));
        QTimer::singleShot(resTimeShow, ui->widgetConnectionMessage, SLOT(hide()));
    }
    else
    {
        ui->labelConnectionMessage->setText(tr("Не удалось выполнить подключение"));
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
    ui->tabWidget->setTabEnabled(1, true);
    ui->widgetPlot->graph(0)->clearData();
    ui->widgetPlot->graph(1)->clearData();
    ui->textConsole->clear();
    ui->widgetPlot->xAxis->setRange(0, 5);
    ui->widgetPlot->replot();
    ui->widgetPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes);
    autosave.stop();
    timer.stop();
    CloseFile();
    WriteSettings();
    ultrasonic.Disconnect();
    disconnect(&timer, SIGNAL(timeout()), 0, 0);
}

void Window::OnStartStop()
{
    bool stopped = ui->actionStartStop->text() == tr("Продолжить");
    QString icon = stopped ? resIconPause : resIconRun;
    QString text = stopped ? tr("Пауза") : tr("Продолжить");
    QCP::Interactions interactions = stopped ? (QCP::iSelectAxes) : (QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes);
    ui->actionStartStop->setIcon(QIcon(icon));
    ui->actionStartStop->setText(text);
    ui->widgetPlot->setInteractions(interactions);
}

void Window::OnAutosave()
{
    if(save.isEmpty()) return;
    WriteFile();
}

void Window::OnPathChanged()
{
    if(!LoadHistory())
        QMessageBox::warning(this, tr("Предупреждение"), tr("Не удалось создать папку журнала. Попробуйте задать её самостоятельно."));
}

void Window::OnTreeExpanded(QTreeWidgetItem* item)
{
    if(!item || !item->childCount()) return;
    item->setIcon(0, QIcon(resIconOpen));
}

void Window::OnTreeCollapsed(QTreeWidgetItem* item)
{
    if(!item || !item->childCount()) return;
    item->setIcon(0, QIcon(resIconClosed));
}

void Window::OnTreeDoubleClicked(QTreeWidgetItem* item)
{
    if(!item || item->childCount()) return;
    if(!ReadFile(item))
        QMessageBox::warning(this, tr("Предупреждение"), tr("Не удалось открыть файл для чтения."));
}

void Window::OnRender()
{
    if(render.isEmpty()) return;
    qreal key = time.elapsed() / 1000.0, value = render.last();
    save.append(key);
    save.append(value);
    ui->textConsole->appendPlainText(QString::number(value));
    ui->widgetPlot->graph(0)->addData(key, value);
    ui->widgetPlot->graph(1)->clearData();
    ui->widgetPlot->graph(1)->addData(key, value);
    ui->widgetPlot->graph(0)->rescaleValueAxis();
    ui->widgetPlot->xAxis->setRange(key + 0.25, 8, Qt::AlignRight);
    ui->widgetPlot->replot();
    render.clear();
}

void Window::OnAxisNameChange(QCPAxis* axis, QCPAxis::SelectablePart part)
{
    if(part != QCPAxis::spAxisLabel) return;
    bool ok;
    QString label = QInputDialog::getText(this, tr("Изменение названия"), tr("Новое название:"), QLineEdit::Normal, axis->label(), &ok, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    if(!ok) return;
    axis->setLabel(label);
    ui->widgetPlot->replot();
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
    if(ui->actionStartStop->text() == tr("Продолжить")) return;
    QList<QByteArray> list = data.split(' ');
    foreach (QByteArray array, list)
    {
        QByteArray trimmed = array.trimmed();
        if(trimmed == "") continue;
        double value = trimmed.toDouble();
        render.append(value);
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

void Window::closeEvent(QCloseEvent* event)
{
    OnDisconnect();
    event->accept();
}

bool Window::ReadSettings()
{
    QSettings settings(resTextApplication);
    settings.beginGroup(resTextGroup);
    ui->editPath->setText(settings.value(resTextPath, QString()).toString());
    settings.endGroup();
    return LoadHistory();
}

bool Window::WriteSettings()
{
    QSettings settings(resTextApplication);
    settings.beginGroup(resTextGroup);
    settings.setValue(resTextPath, ui->editPath->text());
    settings.endGroup();
    return true;
}

bool Window::OpenFile()
{
    QDir directory(home);
    if(!directory.isReadable() || !directory.isAbsolute()) return false;
    QString currentDate = QDate::currentDate().toString("dd.MM.yyyy");
    QString currentTime = QTime::currentTime().toString("HH.mm.ss");
    if(currentDate.isEmpty() || currentTime.isEmpty()) return false;
    directory.mkdir(currentDate);
    if(!directory.cd(currentDate)) return false;
    file.close();
    file.setFileName(directory.absoluteFilePath(currentDate + "#" + currentTime));
    if(file.exists()) return false;
    return true;
}

bool Window::CloseFile()
{
    WriteFile();
    if(file.fileName().isEmpty()) return true;
    if(!file.exists()) return true;
    file.close();
    QFileInfo fileInfo(file);
    QString fileName = fileInfo.fileName();
    QString filePath = fileInfo.absolutePath();
    if(fileName.isEmpty() || filePath.isEmpty()) return false;
    QStringList parts = fileName.split("#");
    if(parts.size() != 2) return false;
    QString dateString = parts.at(0);
    QString timeString = parts.at(1);
    QString currentTime = QTime::currentTime().toString("HH.mm.ss");
    if(dateString.isEmpty() || timeString.isEmpty() || currentTime.isEmpty()) return false;
    QString newFileName = timeString + "-" + currentTime;
    QDir fileDir(filePath);
    file.rename(fileDir.absoluteFilePath(newFileName));
    QList<QTreeWidgetItem*> items = ui->treeHistory->findItems(dateString, Qt::MatchExactly);
    QTreeWidgetItem* dirItem;
    QTreeWidgetItem* fileItem;
    bool isNew = false;
    if(items.size() > 1) return false;
    else if(items.size() == 1) dirItem = items.first();
    else dirItem = new (std::nothrow) QTreeWidgetItem(ui->treeHistory), isNew = true;
    if(!dirItem) return false;
    dirItem->setText(0, dateString);
    dirItem->setIcon(0, QIcon(resIconClosed));
    fileItem = new (std::nothrow) QTreeWidgetItem(dirItem);
    if(!fileItem)
    {
        delete dirItem;
        dirItem = nullptr;
        return false;
    }
    fileItem->setText(0, newFileName.replace('.', ':'));
    fileItem->setIcon(0, QIcon(resIconDocument));
    dirItem->addChild(fileItem);
    if(isNew) ui->treeHistory->addTopLevelItem(dirItem);
    return true;
}

bool Window::ReadFile(QTreeWidgetItem* fileItem)
{
    if(!fileItem) return false;
    QTreeWidgetItem* directoryItem = fileItem->parent();
    if(!directoryItem) return false;
    QString filename = fileItem->text(0);
    QString dirname = directoryItem->text(0);
    if(filename.isEmpty() || dirname.isEmpty()) return false;
    QDir directory(home);
    if(!directory.isReadable() || !directory.isAbsolute() || !directory.cd(dirname)) return false;
    filename.replace(':', '.');
    QFile file(directory.absoluteFilePath(filename));
    if(!file.exists() || !file.open(QIODevice::ReadOnly)) return false;
    QVector<qreal> keys, values;
    QDataStream data(&file);
    if(data.status() != QDataStream::Ok) return false;
    bool isKey = true;
    while(true)
    {
        if(data.atEnd()) break;
        if(data.status() != QDataStream::Ok) return false;
        qreal v;
        if(isKey)
        {
            data >> v;
            keys.append(v);
        }
        else
        {
            data >> v;
            values.append(v);
        }
        isKey = !isKey;
    }
    while(keys.size() > values.size()) keys.pop_back();
    while(values.size() > keys.size()) values.pop_back();
    qreal lastKey = keys.last();
    qreal lastValue = values.last();
    ui->textConsole->clear();
    for(int i = 0; i < values.size(); ++i)
        ui->textConsole->appendPlainText(QString::number(values.at(i)));
    ui->widgetPlot->graph(0)->clearData();
    ui->widgetPlot->graph(1)->clearData();
    ui->widgetPlot->graph(0)->addData(keys, values);
    ui->widgetPlot->graph(1)->addData(lastKey, lastValue);
    ui->widgetPlot->graph(0)->rescaleValueAxis();
    ui->widgetPlot->xAxis->setRange(lastKey + 0.25, 8, Qt::AlignRight);
    ui->widgetPlot->replot();
    return true;
}

bool Window::WriteFile()
{
    file.open(QIODevice::Append);
    if(!file.isOpen() || !file.isWritable() || save.isEmpty())
    {
        save.clear();
        return false;
    }
    QDataStream stream(&file);
    if(stream.status() != QDataStream::Ok)
    {
        save.clear();
        return false;
    }
    for(int i = 0; i < save.size(); ++i)
    {
        if(stream.status() != QDataStream::Ok) break;
        stream << save.at(i);
    }
    file.flush();
    file.close();
    save.clear();
    return true;
}

bool Window::LoadHistory()
{
    if(!home.compare(ui->editPath->text()) && !ui->editPath->text().isEmpty()) return true;
    ui->treeHistory->clear();
    QString path = ui->editPath->text();
    QDir directory(path);
    if(path.isEmpty() || !directory.isReadable() || !directory.isAbsolute())
    {
        path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        directory.setPath(path);
        if(path.isEmpty() || !directory.isReadable() || !directory.isAbsolute())
        {
            ui->editPath->setText(QString());
            return false;
        }
        directory.mkdir(resTextApplication);
        if(!directory.cd(resTextApplication))
        {
            ui->editPath->setText(QString());
            return false;
        }
        path = directory.absolutePath();
        ui->editPath->setText(path);
    }
    home = ui->editPath->text();
    QDir::Filters dirFilters = QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot | QDir::Readable;
    QDir::Filters fileFilters = QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot | QDir::Readable;
    QRegExp dirRegexp("^([0-1][1-9]|2[0-9]|3[0-1])\\.(0[1-9]|1[0-2])\\.(2[0-9][0-9][0-9])");
    QRegExp fileRegexp("^([0-1][0-9]|2[0-3])\\.([0-5][0-9])\\.([0-5][0-9])\\-([0-1][0-9]|2[0-3])\\.([0-5][0-9])\\.([0-5][0-9])$");

    QStringList dirs = directory.entryList(dirFilters);
    foreach(QString dir, dirs)
    {
        if(!dirRegexp.exactMatch(dir)) continue;
        if(!directory.cd(dir)) continue;
        bool appended = false;
        QTreeWidgetItem* treeDir;
        QStringList files = directory.entryList(fileFilters);
        foreach(QString file, files)
        {
            if(!fileRegexp.exactMatch(file)) continue;
            if(!appended)
            {
                treeDir = new (std::nothrow) QTreeWidgetItem(ui->treeHistory);
                if(!treeDir) continue;
                treeDir->setText(0, dir);
                treeDir->setIcon(0, QIcon(resIconClosed));
                appended = true;
            }
            file.replace('.', ':');
            QTreeWidgetItem* treeFile = new (std::nothrow) QTreeWidgetItem(treeDir);
            if(!treeFile) continue;
            treeFile->setText(0, file);
            treeFile->setIcon(0, QIcon(resIconDocument));
            treeDir->addChild(treeFile);
        }
        if(treeDir && !files.isEmpty() && treeDir->childCount() == 0)
        {
            delete treeDir;
            treeDir = nullptr;
        }
        if(ui->treeHistory && treeDir) ui->treeHistory->insertTopLevelItem(0, treeDir);
        directory.cdUp();
    }
    return true;
}