#ifndef UPDATECLIENTDIALOG_H
#define UPDATECLIENTDIALOG_H

#include <QDialog>
#include <QLabel>  // For the QR code label
#include <QImage>  // For handling QImage

namespace Ui {
class updateclientdialog;
}

class updateclientdialog : public QDialog
{
    Q_OBJECT

public:
    explicit updateclientdialog(const QString &id, const QString &nom, const QString &adresse, const QString &contact, QWidget *parent = nullptr);
    ~updateclientdialog();

    QString getNom() const;
    QString getAdresse() const;
    QString getContact() const;

private:
    Ui::updateclientdialog *ui;
    QLabel *qrCodeLabel;  // Label to display the QR code

    void updateQRCode();  // Slot to update the QR code
    void generateQRCode();  // Method to generate the QR code
    QString clientId;  // Client ID (passed in constructor)
    QLabel *m_qrCodeDisplay;


};

#endif // UPDATECLIENTDIALOG_H
