#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <QtSerialPort>

class Ultrasonic : public QObject
{
    Q_OBJECT

public:
    explicit Ultrasonic();
    virtual ~Ultrasonic();

    QStringList GetAvailablePorts();
    bool Connect(const QString&);
    void Disconnect();

signals:
    void OnData(const QByteArray&);

private slots:
    void OnReady();

private:
    QSerialPort port;
};

#endif