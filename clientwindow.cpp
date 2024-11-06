#include "clientwindow.h"
#include "ui_clientwindow.h"
#include <QMessageBox>
#include <QSqlQuery>

clientwindow::clientwindow(Connection &conn, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::clientwindow),
    connection(conn)  // Initialize the connection object by reference
{
    ui->setupUi(this);
    // Remove the manual connection since it's likely already connected in the UI file
    // The connection is probably already set up in the .ui file by Qt Designer
}

clientwindow::~clientwindow()
{
    delete ui;
}

// Method to create a new client in the database
void clientwindow::createClient()
{
    // Get the text from the input fields
    QString nom = ui->nomTF->text();
    QString contact = ui->contactTF->text();
    QString adresse = ui->adresseTF->text();

    // Check if fields are not empty
    if (nom.isEmpty() || contact.isEmpty() || adresse.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please fill in all fields.");
        return;
    }

    // Create SQL query to insert the new client
    QSqlQuery query;
    query.prepare("INSERT INTO CLIENT (NOM, CONTACT, ADRESSE) VALUES (:NOM, :CONTACT, :ADRESSE)");
    query.bindValue(":NOM", nom);
    query.bindValue(":CONTACT", contact);
    query.bindValue(":ADRESSE", adresse);

    // Execute the query and check for success
    if (query.exec()) {
        QMessageBox::information(this, "Success", "Client added successfully!");
        // Clear the input fields after successful insertion
        ui->nomTF->clear();
        ui->contactTF->clear();
        ui->adresseTF->clear();
    } else {
        QMessageBox::critical(this, "Database Error", "Failed to insert client: " + query.lastError().text());
    }
}

// Slot to handle the action when createButton is clicked
void clientwindow::on_createButton_clicked()
{
    createClient();
}
