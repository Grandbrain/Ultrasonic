#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>

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

public slots:
    void OnAbout();

private:
    Ui::Window *ui;
};

#endif