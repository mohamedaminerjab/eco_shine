#ifndef UPDATECLIENTDIALOG_H
#define UPDATECLIENTDIALOG_H

#include <QDialog>

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
};

#endif // UPDATECLIENTDIALOG_H
