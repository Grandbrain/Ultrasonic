#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <QtSerialPort>

class Ultrasonic : public QObject
{
    Q_OBJECT

public:
    explicit Ultrasonic();
    virtual ~Ultrasonic();

signals:

public slots:

private:
    QSerialPort port;
};

#endif