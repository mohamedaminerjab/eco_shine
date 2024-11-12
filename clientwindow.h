#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QDialog>
#include <QSqlTableModel>
#include "connection.h"
#include "updateclientdialog.h"
#include <QtCharts>
#include <QtCharts/QPieSeries>
#include <QtCharts/QChartView>

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
    void on_clientTableView_doubleClicked(const QModelIndex &index);
    void on_deleteButton_clicked();
    void on_searchLineEdit_textChanged(const QString &text);
    void on_pdfButton_clicked();






private:
    Ui::clientwindow *ui;
    Connection &connection;
    QSqlTableModel *model;
    void updateStatsChart();
    QChart *statsChart;
    QChartView *chartView;
};

#endif // CLIENTWINDOW_H
