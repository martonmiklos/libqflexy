#include "ecr.h"
#include <QDebug>

void ECRCommand::createRequest(QString command, QStringList parameters)
{
    QString paramString = parameters.join('\t');
    if (!paramString.isEmpty())
        paramString.prepend('\t');
    m_request = QString("%1\tREQ%2\n").arg(command).arg(paramString);
}

ECRConnectCommand::ECRConnectCommand() :
    ECRCommand(ECRCommandTypeConnect)
{
    createRequest("CONNECT");
}

bool ECRConnectCommand::processAnswer(QString answer)
{
    QStringList answerParts = answer.trimmed().split('\t');
    if (answerParts.size() != 3) {
        errorString = QObject::tr("Parameter count mismatch to the CONNECT command");
        return false;
    }
    deviceState = answerParts.at(2).toInt();
    return true;
}

ECRDisConnectCommand::ECRDisConnectCommand() :
    ECRCommand(ECRCommandTypeDisConnect)
{
    createRequest("DISCONNECT");
}

ECRGetDevInfoCommand::ECRGetDevInfoCommand() :
    ECRCommand(ECRCommandTypeGetDevInfo)
{
    createRequest("GETDEVINFO");
}

ECRGetDevStateCommand::ECRGetDevStateCommand() :
    ECRCommand(ECRCommandTypeGetDevInfo)
{
    createRequest("GETDEVSTATE");
}

ECR::ECR(QObject *parent, QIODevice *ioDevice) :
    QObject(parent),
    m_currentState(ECRDisconnecting),
    m_ioDevice(ioDevice)
{
}

void ECR::setDevice(QIODevice *device)
{
    m_ioDevice = device;
    connect(m_ioDevice, SIGNAL(readyRead()), this, SLOT(deviceReadyRead()));
}

QString ECR::errorString(quint32 errorCode) const
{
    switch (errorCode) {
    case 0x00000000: return tr("Operation done");
    case 0x00050002: return tr("Report warning limit reached, less then 60 financial Z reports are available");
    case 0x0006000B: return tr("Wrong parameter value");
    case 0x010A001A: return tr("Journal warning limit. Journal used on 80%.");
    case 0x010A001B: return tr("Journal warning limit. Journal used on 95%.");
    case 0x80010002: return tr("Record ID is out of range");
    case 0x80020005: return tr("Missing parameter");
    case 0x80040006: return tr("Purchase is full, end receipt");
    case 0x80040009: return tr("No such item");
    case 0x8004000B: return tr("The last possible payment does not settle retail transaction, realize last payment");
    case 0x8004000E: return tr("Value adjustment limit");
    case 0x8004000F: return tr("Percent adjustment limit");
    case 0x80040010: return tr("Zero or negative value of subtotal - at subtotal adjust operation");
    case 0x80040011: return tr("Price already adjusted");
    case 0x80040012: return tr("Not adjustable item");
    case 0x80040013: return tr("Quantity1 is out of range");
    case 0x80040014: return tr("Quantity2 is out of range");
    case 0x80040015: return tr("Total quantity (q1 * q2) is out of range");
    case 0x80040016: return tr("Invalid quantity");
    case 0x80040017: return tr("Unit price is out of range");
    case 0x80040018: return tr("Invalid unit price");
    case 0x80040019: return tr("Purchase is in payment");
    case 0x8004001C: return tr("Not enough currency");
    case 0x8004001E: return tr("Sale impossible inactive PLU");
    case 0x8004001F: return tr("Unit price change is not allowed");
    case 0x80040020: return tr("Item not found");
    case 0x80040021: return tr("Zero total price of PLU");
    case 0x80040022: return tr("No sale yet");
    case 0x80040023: return tr("Operation is not possible in this state");
    case 0x80040025: return tr("Selected item is not returnable container");
    case 0x80040026: return tr("Link PLU is not returnable container");
    case 0x80040028: return tr("Illegal tender");
    case 0x80040030: return tr("Item void is not allowed by reason of subtotal adjustment");
    case 0x8004003A: return tr("Void of descriptive PLU is not allowed");
    case 0x8004003B: return tr("Sale of descriptive PLU is not allowed");
    case 0x8004003C: return tr("Bad tender value");
    case 0x8004003F: return tr("Invalid quantity1 value");
    case 0x80040040: return tr("Invalid quantity2 value");
    case 0x80040041: return tr("Invalid payment value");
    case 0x80040042: return tr("Too large payment value");
    case 0x80040043: return tr("Change too large");
    case 0x80040044: return tr("No item in the purchase");
    case 0x80040049: return tr("Not enough foreign currency");
    case 0x8005000A: return tr("Unable to execute daily report, report already done");
    case 0x8005000B: return tr("Unable to execute monthly report, no daily report was done");
    case 0x8005000C: return tr("Unable to execute monthly report, report already done");
    case 0x8005000E: return tr("Autoexport txt journal error");
    case 0x80060007: return tr("Unknown record");
    case 0x80060009: return tr("Unsupported table");
    case 0x8006000A: return tr("Unknown record attribute");
    case 0x8006000C: return tr("Value cannot be set (read-only attribute or the change requirements are not fulfilled )");
    case 0x80060012: return tr("Value cannot be set daily report required");
    case 0x80060013: return tr("Value cannot be set, monthly report required");
    case 0x80060014: return tr("Operation cannot be executed (requirements are not fulfilled)");
    case 0x80060018: return tr("Value cannot be set, bar code duplicity");
    case 0x80060019: return tr("Unsupported or bad record value");
    case 0x8006001A: return tr("No graphic logo");
    case 0x8006001B: return tr("Header was not set");
    case 0x80070007: return tr("Unsupported operation");
    case 0x80070009: return tr("ECR is busy");
    case 0x8007000A: return tr("Connection refused");
    case 0x8101006A: return tr("Illegal or unsupported operation, or invalid parameter value");
    case 0x810100C9: return tr("Cover is open");
    case 0x810100CA: return tr("Missing journal paper");
    case 0x810100CB: return tr("Missing receipt paper");
    case 0x810100D8: return tr("Receipt total overflow");
    case 0x810100DC: return tr("Negative total (in one tax group at least) – in the case of first payment");
    default: return tr("Unknown return code: 0x%1").arg(QString::number(errorCode, 16).rightJustified(8, '0'));
    }
}

bool ECR::connectToECR()
{
    if (!ioDeviceReady())
        return false;

    emit stateChanged(ECRConnecting, m_currentState);
    m_currentState = ECRConnecting;
    return true;
}

bool ECR::disconnectFromECR()
{
    if (!ioDeviceReady())
        return false;

    if (m_currentState != ECRDisconnected) {
        emit stateChanged(ECRDisconnecting, m_currentState);
    }
    return true;
}

bool ECR::sendCommand(ECRCommand *command)
{
    if (!ioDeviceReady()) {
        return false;
    }

    if (m_commandQueue.isEmpty()) {
        m_currentCommand = command;
        m_ioDevice->write(command->m_request.toLocal8Bit());

    } else {
        m_commandQueue.append(command);
    }
    return true;
}

bool ECR::sendNextCommand()
{
    if (!ioDeviceReady()) {
        return false;
    }

    if (!m_commandQueue.isEmpty()) {
        m_currentCommand = m_commandQueue.takeFirst();
        return sendCommand(m_currentCommand);
    }
    return false;
}

void ECR::changeState(ECRState newState)
{
    emit stateChanged(newState, m_currentState);
    m_currentState = newState;
}

void ECR::deviceReadyRead()
{
    while (m_ioDevice->canReadLine()) {
        if (m_currentCommand != NULL) {
            if (m_currentCommand->processAnswer(m_ioDevice->readLine())) {
                switch (m_currentCommand->commandType()) {
                case ECRCommand::ECRCommandTypeConnect:
                    ECRConnectCommand *cmd = dynamic_cast<ECRConnectCommand*>(m_currentCommand);
                    changeState(cmd->deviceState?ECRDisconnected:ECRConnected);
                    break;
                }

                delete m_currentCommand;
                m_currentCommand = NULL;
                sendNextCommand();
            }
        } else {
            qDebug() << "data receieved but no command sent";
        }
    }
}

bool ECR::ioDeviceReady()
{
    if (m_ioDevice == NULL)
        return false;

    if (!m_ioDevice->isOpen())
        return false;

    return true;
}

void ECR::timeOutSlot()
{

}

QString ECR::reportPeroidToParameter(ReportPeriod period)
{
    switch (period) {
    default:
    case ReportDaily: return "DAILY";
    case ReportMonthly: return "MONTHLY";
    }
}
