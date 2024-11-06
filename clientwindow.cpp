#include "clientwindow.h"
#include "ui_clientwindow.h"
#include "updateclientdialog.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QSqlError>
#include <QDebug>

clientwindow::clientwindow(Connection &conn, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::clientwindow),
    connection(conn)
{
    ui->setupUi(this);

    // Initialize the model with the correct database connection
    model = new QSqlTableModel(this, QSqlDatabase::database());
    model->setTable("client");  // Try lowercase table name

    // Check if the table exists and is accessible
    if (!model->select()) {
        qDebug() << "Error loading table:" << model->lastError().text();
        // Try with uppercase table name
        model->setTable("CLIENT");
        if (!model->select()) {
            qDebug() << "Error loading table (uppercase):" << model->lastError().text();
        }
    }

    model->setEditStrategy(QSqlTableModel::OnManualSubmit);

    // Set the headers
    model->setHeaderData(0, Qt::Horizontal, tr("NOM"));
    model->setHeaderData(1, Qt::Horizontal, tr("CONTACT"));
    model->setHeaderData(2, Qt::Horizontal, tr("IDENTIFIANT"));
    model->setHeaderData(3, Qt::Horizontal, tr("ADRESSE"));

    // Set up the table view
    ui->clientTableView->setModel(model);
    ui->clientTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->clientTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->clientTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Make columns stretch to fill the available space
    ui->clientTableView->horizontalHeader()->setStretchLastSection(true);
    ui->clientTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Debug: Print the number of rows in the model
    qDebug() << "Number of rows in model:" << model->rowCount();

    // Run a test query to verify data exists
    QSqlQuery testQuery("SELECT * FROM CLIENT");
    if (testQuery.exec()) {
        int count = 0;
        while (testQuery.next()) {
            count++;
        }
        qDebug() << "Number of records in direct query:" << count;
    } else {
        qDebug() << "Test query failed:" << testQuery.lastError().text();
    }

    // Connect the refresh button
    connect(ui->refreshButton, &QPushButton::clicked, this, &clientwindow::refreshClientList);
}

clientwindow::~clientwindow()
{
    delete ui;
    delete model;
}

void clientwindow::createClient()
{
    QString nom = ui->nomTF->text();
    QString contact = ui->contactTF->text();
    QString adresse = ui->adresseTF->text();

    if (nom.isEmpty() || contact.isEmpty() || adresse.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please fill in all fields.");
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO CLIENT (NOM, ADRESSE, CONTACT) VALUES (:NOM, :ADRESSE, :CONTACT)");
    query.bindValue(":NOM", nom);
    query.bindValue(":ADRESSE", adresse);
    query.bindValue(":CONTACT", contact);

    if (query.exec()) {
        QMessageBox::information(this, "Success", "Client added successfully!");
        // Clear the input fields
        ui->nomTF->clear();
        ui->contactTF->clear();
        ui->adresseTF->clear();
        // Refresh the table view
        refreshClientList();

        // Debug: Verify the new row count
        qDebug() << "After insert, row count:" << model->rowCount();
    } else {
        QMessageBox::critical(this, "Database Error", "Failed to insert client: " + query.lastError().text());
    }
}

void clientwindow::refreshClientList()
{
    // Debug: Print current row count
    qDebug() << "Before refresh, row count:" << model->rowCount();

    // Force a complete reload
    model->setTable("CLIENT");

    if (!model->select()) {
        qDebug() << "Refresh failed:" << model->lastError().text();
        QMessageBox::warning(this, "Refresh Error",
                             "Failed to refresh client list: " + model->lastError().text());
        return;
    }

    // Reset headers after refresh
    model->setHeaderData(0, Qt::Horizontal, tr("NOM"));
    model->setHeaderData(1, Qt::Horizontal, tr("CONTACT"));
    model->setHeaderData(2, Qt::Horizontal, tr("IDENTIFIANT"));
    model->setHeaderData(3, Qt::Horizontal, tr("ADRESSE"));

    // Debug: Print new row count
    qDebug() << "After refresh, row count:" << model->rowCount();
}

void clientwindow::on_createButton_clicked()
{
    createClient();
}

void clientwindow::on_clientTableView_doubleClicked(const QModelIndex &index)
{
    // Get the row data
    QString id = model->data(model->index(index.row(), 2)).toString(); // IDENTIFIANT column
    QString nom = model->data(model->index(index.row(), 0)).toString(); // NOM column
    QString contact = model->data(model->index(index.row(), 1)).toString(); // CONTACT column
    QString adresse = model->data(model->index(index.row(), 3)).toString(); // ADRESSE column

    // Create and show the update dialog
    updateclientdialog updateDialog(id, nom, adresse, contact, this);

    if (updateDialog.exec() == QDialog::Accepted) {
        // Get the updated values
        QString newNom = updateDialog.getNom();
        QString newAdresse = updateDialog.getAdresse();
        QString newContact = updateDialog.getContact();

        // Update the database
        QSqlQuery query;
        query.prepare("UPDATE CLIENT SET NOM = :nom, ADRESSE = :adresse, CONTACT = :contact "
                      "WHERE IDENTIFIANT = :id");
        query.bindValue(":nom", newNom);
        query.bindValue(":adresse", newAdresse);
        query.bindValue(":contact", newContact);
        query.bindValue(":id", id);

        if (query.exec()) {
            QMessageBox::information(this, "Success", "Client updated successfully!");
            refreshClientList(); // Refresh the table view
        } else {
            QMessageBox::critical(this, "Error", "Failed to update client: " + query.lastError().text());
        }
    }
}

void clientwindow::on_deleteButton_clicked()
{
    // Get the current selection
    QModelIndex currentIndex = ui->clientTableView->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "Selection Error", "Please select a client to delete.");
        return;
    }

    // Get the client ID and name for confirmation
    QString id = model->data(model->index(currentIndex.row(), 2)).toString(); // IDENTIFIANT column
    QString nom = model->data(model->index(currentIndex.row(), 0)).toString(); // NOM column

    // Confirm deletion
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirm Deletion",
                                                              "Are you sure you want to delete client: " + nom + "?",
                                                              QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // Delete from database
        QSqlQuery query;
        query.prepare("DELETE FROM CLIENT WHERE IDENTIFIANT = :id");
        query.bindValue(":id", id);

        if (query.exec()) {
            QMessageBox::information(this, "Success", "Client deleted successfully!");
            refreshClientList(); // Refresh the table view
        } else {
            QMessageBox::critical(this, "Error", "Failed to delete client: " + query.lastError().text());
        }
    }
}
