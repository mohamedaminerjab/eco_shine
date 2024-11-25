#include "messagedialog.h"
#include <QDateTime>

MessageDialog::MessageDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Chat Popup Window");
    setMinimumSize(300, 400);

    QVBoxLayout *layout = new QVBoxLayout(this);

    chatHistory = new QTextEdit(this);
    chatHistory->setReadOnly(true);
    layout->addWidget(chatHistory);

    QHBoxLayout *inputLayout = new QHBoxLayout();

    messageInput = new QLineEdit(this);
    sendButton = new QPushButton("Send", this);

    inputLayout->addWidget(messageInput);
    inputLayout->addWidget(sendButton);

    layout->addLayout(inputLayout);

    connect(sendButton, &QPushButton::clicked, this, &MessageDialog::onSendClicked);
    connect(messageInput, &QLineEdit::returnPressed, this, &MessageDialog::onSendClicked);
}

void MessageDialog::onSendClicked()
{
    QString message = messageInput->text().trimmed();
    if (!message.isEmpty()) {
        emit messageSent(message);
        addMessage(message);
        messageInput->clear();
    }
}

void MessageDialog::addMessage(const QString &message, bool fromMain)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString source = fromMain ? "Rjab" : "Aymen";
    QString formattedMessage = QString("[%1] %2: %3").arg(timestamp, source, message);
    chatHistory->append(formattedMessage);
}
