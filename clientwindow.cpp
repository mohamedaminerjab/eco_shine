#include "clientwindow.h"
#include "ui_clientwindow.h"
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
