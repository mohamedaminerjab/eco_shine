#include "clientwindow.h"
#include <QApplication>
#include <QMessageBox>
#include "connection.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Create the connection object
    Connection c;
    bool test = c.createconnect();

    clientwindow cw(c);


    // Try to connect to the database

    if (test) {
        // Create the client window and pass the connection object by reference
        cw.show();
        // Optionally, display the connection success message
        QMessageBox::information(nullptr, QObject::tr("Database is open"),
                                 QObject::tr("Connection successful.\n"
                                             "Click Cancel to exit."), QMessageBox::Cancel);
    } else {
        // Show a critical error message if the connection fails
        QMessageBox::critical(nullptr, QObject::tr("Database is not open"),
                              QObject::tr("Connection failed.\n"
                                          "Click Cancel to exit."), QMessageBox::Cancel);
    }

    return a.exec();  // Start the Qt event loop
}
