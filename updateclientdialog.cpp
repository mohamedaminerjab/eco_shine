#include "updateclientdialog.h"
#include "ui_updateclientdialog.h"

updateclientdialog::updateclientdialog(const QString &id, const QString &nom, const QString &adresse, const QString &contact, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::updateclientdialog)
{
    ui->setupUi(this);
    ui->nomLineEdit->setText(nom);
    ui->adresseLineEdit->setText(adresse);
    ui->contactLineEdit->setText(contact);

    // Connect the button box signals
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

updateclientdialog::~updateclientdialog()
{
    delete ui;
}

QString updateclientdialog::getNom() const
{
    return ui->nomLineEdit->text();
}

QString updateclientdialog::getAdresse() const
{
    return ui->adresseLineEdit->text();
}

QString updateclientdialog::getContact() const
{
    return ui->contactLineEdit->text();
}
