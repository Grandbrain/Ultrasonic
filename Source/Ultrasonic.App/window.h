#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "ultrasonic.h"
#include "qcustomplot.h"

namespace Ui
{
    class Window;
}

class Window : public QMainWindow
{
    Q_OBJECT

public:
    explicit Window(QWidget* = 0);
    virtual ~Window();

private slots:
    void OnAbout();
    void OnRender();
    void OnConnect();
    void OnAutosave();
    void OnExplorer();
    void OnStartStop();
    void OnMouseWheel();
    void OnDisconnect();
    void OnPathChanged();
    void OnData(const QByteArray&);
    void OnTreeExpanded(QTreeWidgetItem*);
    void OnTreeCollapsed(QTreeWidgetItem*);
    void OnTreeDoubleClicked(QTreeWidgetItem*);
    void OnAxisNameChange(QCPAxis*, QCPAxis::SelectablePart);

private:
    bool eventFilter(QObject*, QEvent*);
    void closeEvent(QCloseEvent*);
    bool OpenFile();
    bool CloseFile();
    bool WriteFile();
    bool LoadHistory();
    bool ReadSettings();
    bool WriteSettings();
    bool ReadFile(QTreeWidgetItem*);

private:
    Ui::Window *ui;
    QFile file;
    QTime time;
    QTimer timer;
    QString home;
    QTimer autosave;
    QVector<qreal> save;
    QVector<qreal> render;
    Ultrasonic ultrasonic;
};

#endif