#ifndef MESSAGEDIALOG_H
#define MESSAGEDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>

class MessageDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MessageDialog(QWidget *parent = nullptr);
    void addMessage(const QString &message, bool fromMain = false);

signals:
    void messageSent(const QString &message);

private slots:
    void onSendClicked();

private:
    QTextEdit *chatHistory;
    QLineEdit *messageInput;
    QPushButton *sendButton;
};

#endif // MESSAGEDIALOG_H
