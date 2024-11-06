#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QDialog>
#include "connection.h"  // Include the connection header

namespace Ui {
class clientwindow;
}

class clientwindow : public QDialog
{
    Q_OBJECT

public:
    explicit clientwindow(Connection &conn, QWidget *parent = nullptr);  // Pass connection by reference here
    ~clientwindow();

private:
    Ui::clientwindow *ui;
    Connection &connection;  // Store the connection object by reference

    void createClient();  // Method to create a new client in the database

private slots:
    void on_createButton_clicked();
};

#endif // CLIENTWINDOW_H
