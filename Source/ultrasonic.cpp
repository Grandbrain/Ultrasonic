#include "ultrasonic.h"

Ultrasonic::Ultrasonic()
{
    connect(&port, SIGNAL(readyRead()), SLOT(OnReady()));
}

Ultrasonic::~Ultrasonic()
{

}

QStringList Ultrasonic::GetAvailablePorts()
{
    QStringList list;
    foreach (const QSerialPortInfo& info, QSerialPortInfo::availablePorts())
    {
        QString portName = info.portName();
        if(!info.description().isEmpty()) portName += " | " + info.description();
        list.append(portName);
    }
    return list;
}

bool Ultrasonic::Connect(const QString& portName)
{
    port.close();
    port.setPortName(portName.split("|").at(0));
    if(!port.open(QIODevice::ReadWrite)) return false;
    return port.setDataTerminalReady(true);
}

void Ultrasonic::Disconnect()
{
    port.close();
}

void Ultrasonic::OnReady()
{
    emit OnData(port.readAll());
}