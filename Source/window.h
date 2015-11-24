#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QTime>
#include "ultrasonic.h"

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
    void OnConnect();
    void OnDisconnect();
    void OnRender();
    void OnStartStop();
    void OnAutosaveChange();
    void OnAutosave();
    void OnAxisNameChange();
    void OnLegendNameChange();
    void OnMouseWheel();
    void OnData(const QByteArray&);

private:
    bool eventFilter(QObject*, QEvent*);

private:
    Ui::Window *ui;
    QVector<qreal> array;
    QVector<qreal> saveArray;
    QTimer renderTimer;
    QTimer saveTimer;
    QTime time;
    Ultrasonic ultrasonic;
};

#endif