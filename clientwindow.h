#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QDialog>
#include <QSqlTableModel>
#include "connection.h"

namespace Ui {
class clientwindow;
}

class clientwindow : public QDialog
{
    Q_OBJECT

public:
    explicit clientwindow(Connection &conn, QWidget *parent = nullptr);
    ~clientwindow();

private slots:
    void on_createButton_clicked();
    void createClient();
    void refreshClientList();

private:
    Ui::clientwindow *ui;
    Connection &connection;
    QSqlTableModel *model;
};

#endif // CLIENTWINDOW_H
