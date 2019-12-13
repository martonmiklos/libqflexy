#ifndef ECR_H
#define ECR_H

#include "libqflexy_global.h"

#include <QIODevice>
#include <QObject>
#include <QTimer>
#include <QString>
#include <QStringList>

class LIBQFLEXYSHARED_EXPORT ECRCommand
{
    friend class ECR;
public:
    enum ECRCommandType {
        ECRCommandTypeConnect,
        ECRCommandTypeDisConnect,
        ECRCommandTypeGetDevInfo,
        ECRCommandTypeGetDevState
    };

    ECRCommand(ECRCommandType type);
    virtual ~ECRCommand() = 0;
    ECRCommandType commandType() const {return m_commandType;}
    virtual bool processAnswer(QString answer) = 0;
    QString errorString;

protected:
    ECRCommandType m_commandType;
    QString m_request;
    QStringList responses;

    void createRequest(QString command, QStringList parameters = QStringList());
};

class ECRConnectCommand : public ECRCommand
{
public:
    ECRConnectCommand();
    bool processAnswer(QString answer);
    quint32 deviceState;

private:
    quint32 m_deviceState;
};

class ECRDisConnectCommand : public ECRCommand
{
public:
    ECRDisConnectCommand();
};

class ECRGetDevInfoCommand : public ECRCommand
{
public:
    ECRGetDevInfoCommand();
};

class ECRGetDevStateCommand : public ECRCommand
{
public:
    ECRGetDevStateCommand();
};

class LIBQFLEXYSHARED_EXPORT ECR : public QObject
{
    friend class ECRCommand;
public:
    enum ECRState {
        ECRDisconnected,
        ECRConnecting,
        ECRConnected,
        ECRConnectionBroken,
        ECRDisconnecting
    };

    enum ECRBacklightMode {
        ECRBacklightOn = 0,
        ECRBacklightOff = 1,
        ECRBacklightAutoOff = 2
    };

    enum ReportPeriod {
        ReportDaily,
        ReportMonthly
    };

    ECR(QObject *parent = 0, QIODevice *ioDevice = NULL);

    /**
     * @brief setDevice
     * @param device
     * @
     */
    void setDevice(QIODevice *device);

    QString errorString(quint32 errorCode) const;

    quint32 currentState();

    bool connected();

    bool setECRDateTime(QDateTime dateTime);
    bool syncECRDateTime(QDateTime dateTime);

    quint16 standByTime();
    bool setStandByTime(quint16 standByTime);

    bool standByEnabled();
    bool setStandByEnabled(bool enabled);

    ECRBacklightMode backlightMode();
    bool setECRBackLightMode(ECRBacklightMode mode);

    QString deviceType() const {return m_deviceType;}


    bool connectToECR();
    bool disconnectFromECR();

    // Report generating commands
    bool makeFiscalReport();
    bool makeCashierReport(ReportPeriod period, bool printReport);
    bool makeDepartmentReport(ReportPeriod period, bool printReport);
    bool makePLUReport(bool printReport, quint32 firstID, quint32 lastID);
    bool makeDrawerReport(ReportPeriod period, bool printReport);

    // Table manipulation commands


signals:
    void stateChanged(ECRState newState, ECRState oldState);

private:
    ECRState m_currentState;
    QIODevice *m_ioDevice;

    void stopTimeOutTimer();
    QString m_deviceType, m_countryID, m_swVersion, m_protocolVersion, m_serialNumber;
    bool m_isFiscal;

    ECRCommand *m_currentCommand;
    QList<ECRCommand*> m_commandQueue;
    bool sendCommand(ECRCommand *command);
    bool sendNextCommand();
    void changeState(ECRState newState);

private slots:
    void deviceReadyRead();
    bool ioDeviceReady();

    void timeOutSlot();

    QString reportPeroidToParameter(ReportPeriod period);
};

#endif // ECR_H
