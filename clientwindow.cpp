#include "clientwindow.h"
#include "ui_clientwindow.h"
#include "updateclientdialog.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QSqlError>
#include <QDebug>
#include <QtCharts/QPieSeries>
#include <QtCharts/QChartView>
#include <QPrinter>
#include <QPainter>
#include <QFileDialog>
#include <QDateTime>


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

    // Enable sorting
    ui->clientTableView->setSortingEnabled(true);
    model->setSort(0, Qt::AscendingOrder); // Initially sort by first column (NOM)

    // Make columns stretch to fill the available space
    ui->clientTableView->horizontalHeader()->setStretchLastSection(true);
    ui->clientTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    connect(ui->searchLineEdit, &QLineEdit::textChanged, this, &clientwindow::on_searchLineEdit_textChanged);

    // Initialize chart components
    statsChart = new QChart();
    chartView = new QChartView(statsChart);
    chartView->setRenderHint(QPainter::Antialiasing);

    // Find the statTab and set up its layout
    if (QWidget* statTab = ui->statTab) {  // Changed from tabWidget->widget(1) to ui->statTab
        QVBoxLayout* layout = new QVBoxLayout(statTab);
        layout->addWidget(chartView);
        statTab->setLayout(layout);
    } else {
        qDebug() << "Failed to find statTab";
    }

    // Update the stats initially
    updateStatsChart();

    // Connect refresh button to also update stats
    connect(ui->refreshButton, &QPushButton::clicked, this, &clientwindow::updateStatsChart);

    // Connect tab widget's currentChanged signal to update stats when switching to stats tab
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, [this](int index) {
        if (ui->tabWidget->widget(index) == ui->statTab) {  // Changed to check for statTab directly
            updateStatsChart();
        }
    });

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

    connect(ui->refreshButton, &QPushButton::clicked, this, &clientwindow::refreshClientList);
    connect(ui->pdfButton, &QPushButton::clicked, this, &clientwindow::on_pdfButton_clicked);

    // Initialize the rating system
    currentRating = 0;


    // Find tab_3 and set up its layout
    if (QWidget* reviewTab = ui->tab_3) {
        QVBoxLayout* mainLayout = new QVBoxLayout(reviewTab);

        // Create horizontal layout for stars
        QHBoxLayout* starsLayout = new QHBoxLayout();

        // Create star buttons
        for (int i = 0; i < 5; i++) {
            QPushButton* starButton = new QPushButton("☆");
            starButton->setFixedSize(40, 40);
            starButton->setStyleSheet(
                "QPushButton { font-size: 24px; border: none; } "
                "QPushButton:hover { color: gold; }");

            // Connect each button
            connect(starButton, &QPushButton::clicked, this, [this, i]() {
                updateStarRating(i + 1);
            });

            starsLayout->addWidget(starButton);
            starButtons.append(starButton);
        }

        // Add stretches for centering
        starsLayout->insertStretch(0);
        starsLayout->addStretch();

        // Create description text edit
        QTextEdit* descriptionEdit = new QTextEdit();
        descriptionEdit->setPlaceholderText("Enter your review here...");
        descriptionEdit->setMaximumHeight(100);
        descriptionEdit->setObjectName("descriptionEdit");

        // Create submit button
        QPushButton* submitButton = new QPushButton("Submit Review");
        submitButton->setObjectName("submitReviewButton");
        connect(submitButton, &QPushButton::clicked, this, &clientwindow::on_submitReviewButton_clicked);

        // Add widgets to main layout
        mainLayout->addLayout(starsLayout);
        mainLayout->addWidget(descriptionEdit);
        mainLayout->addWidget(submitButton);
        mainLayout->addStretch();
    }

}

clientwindow::~clientwindow()
{
    delete ui;
    delete model;
    delete statsChart;    // Add this
    delete chartView;     // Add this
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
    // Store current filter and sorting
    QString currentFilter = model->filter();
    int currentSortColumn = ui->clientTableView->horizontalHeader()->sortIndicatorSection();
    Qt::SortOrder currentSortOrder = ui->clientTableView->horizontalHeader()->sortIndicatorOrder();

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

    // Reapply the filter if there was one
    if (!currentFilter.isEmpty()) {
        model->setFilter(currentFilter);
    }

    // Reapply the sorting
    model->setSort(currentSortColumn, currentSortOrder);
    model->select();

    // Debug: Print new row count
    qDebug() << "After refresh, row count:" << model->rowCount();

    updateStatsChart();

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

void clientwindow::on_searchLineEdit_textChanged(const QString &text)
{
    // Create the filter condition for multiple columns
    QString filter;
    if (!text.isEmpty()) {
        QStringList filters;
        filters << QString("NOM LIKE '%%1%'").arg(text)
                << QString("CONTACT LIKE '%%1%'").arg(text)
                << QString("ADRESSE LIKE '%%1%'").arg(text)
                << QString("IDENTIFIANT LIKE '%%1%'").arg(text);

        // Combine filters with OR
        filter = filters.join(" OR ");
    }

    // Apply the filter
    model->setFilter(filter);

    // Debug output
    qDebug() << "Applied filter:" << filter;
    qDebug() << "Filtered rows:" << model->rowCount();

    if (model->lastError().isValid()) {
        qDebug() << "Filter error:" << model->lastError().text();
    }
}

void clientwindow::updateStatsChart()
{
    // Clear the old chart
    statsChart->removeAllSeries();

    // Create a new pie series
    QPieSeries *series = new QPieSeries();

    // Query to count clients by address
    QSqlQuery query;
    query.prepare("SELECT ADRESSE, COUNT(*) as count "
                  "FROM CLIENT "
                  "GROUP BY ADRESSE "
                  "ORDER BY count DESC");

    if (query.exec()) {
        // Map to store address counts
        QMap<QString, int> addressCounts;
        int totalClients = 0;

        // Collect the data
        while (query.next()) {
            QString address = query.value("ADRESSE").toString();
            int count = query.value("count").toInt();
            addressCounts[address] = count;
            totalClients += count;
        }

        // Add slices to the pie series
        for (auto it = addressCounts.begin(); it != addressCounts.end(); ++it) {
            QString address = it.key();
            int count = it.value();
            qreal percentage = (count * 100.0) / totalClients;

            // Create slice with percentage in label
            QString label = QString("%1\n%2%\n(%3)")
                                .arg(address)
                                .arg(QString::number(percentage, 'f', 1))
                                .arg(count);

            QPieSlice *slice = series->append(label, count);

            // Set random color for the slice
            slice->setColor(QColor(rand() % 256, rand() % 256, rand() % 256));

            // Make slices slightly exploded
            slice->setExploded(true);
            slice->setExplodeDistanceFactor(0.1);

            // Connect hover signals to show/hide label
            connect(slice, &QPieSlice::hovered, [slice](bool show) {
                slice->setLabelVisible(show);
            });
        }
    } else {
        qDebug() << "Stats query failed:" << query.lastError().text();
    }

    // Add the series to the chart
    statsChart->addSeries(series);

    // Set chart title
    statsChart->setTitle("Client Distribution by Address");

    // Customize the chart
    statsChart->setAnimationOptions(QChart::AllAnimations);
    statsChart->legend()->setVisible(true);
    statsChart->legend()->setAlignment(Qt::AlignRight);

    // Update the view
    chartView->setChart(statsChart);
}




void clientwindow::on_pdfButton_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save PDF"), QString(),
                                                    tr("PDF Files (*.pdf);;All Files (*)"));

    if (fileName.isEmpty())
        return;

    if (!fileName.endsWith(".pdf"))
        fileName += ".pdf";

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageSize(QPageSize(QPageSize::A4));

    QPainter painter(&printer);
    painter.begin(&printer);

    // Fonts and settings
    QFont titleFont("Arial", 12, QFont::Bold);    // Reduced from 16 to 12
    QFont headerFont("Arial", 8, QFont::Bold);    // Reduced from 10 to 8
    QFont contentFont("Arial", 7);                // Reduced from 9 to 7

    // Adjusted margins
    const int margin = 40;
    const int rowHeight = 600;                     // Keeping this as per the original code
    int currentY = margin;

    // Get page width excluding margins
    int pageWidth = printer.pageRect(QPrinter::Point).width() - (2 * margin);

    // Column widths (keeping as per the original code)
    QVector<int> columnWidths = {
        static_cast<int>(pageWidth * 0.20 * 9),  // Name
        static_cast<int>(pageWidth * 0.20 * 9),  // Contact
        static_cast<int>(pageWidth * 0.15 * 9),  // ID
        static_cast<int>(pageWidth * 0.45 * 9)   // Address - increased for longer text
    };

    // Draw title
    painter.setFont(titleFont);
    QRect titleRect(margin, currentY, pageWidth, rowHeight * 1.5);
    painter.drawText(titleRect, Qt::AlignCenter, "Client List");
    currentY += rowHeight * 1.5;

    // Draw date with smaller spacing
    painter.setFont(contentFont);
    QString datetime = QDateTime::currentDateTime().toString("Generated on: yyyy-MM-dd hh:mm:ss");
    painter.drawText(margin, currentY, datetime);
    currentY += rowHeight;

    // Function to draw table row with borders
    auto drawTableRow = [&](const QStringList& rowData, const QFont& font, bool isHeader = false) {
        painter.setFont(font);
        int currentX = margin;

        // Draw horizontal line above
        painter.drawLine(margin, currentY, margin + pageWidth, currentY);

        for (int i = 0; i < rowData.size(); ++i) {
            // Draw cell borders
            QRect cellRect(currentX, currentY, columnWidths[i], rowHeight);
            painter.drawRect(cellRect);

            // Draw text with smaller padding
            QString text = rowData[i];
            QFontMetrics fm(font);
            QString elidedText = fm.elidedText(text, Qt::ElideRight, columnWidths[i] - 4); // Reduced padding

            QRect textRect = cellRect.adjusted(2, 0, -2, 0); // Reduced padding from 5 to 2
            painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, elidedText);

            currentX += columnWidths[i];
        }

        return currentY + rowHeight;
    };

    // Draw headers
    QStringList headers = {"Name", "Contact", "ID", "Address"};
    currentY = drawTableRow(headers, headerFont, true);

    // Draw content
    painter.setFont(contentFont);
    for (int row = 0; row < model->rowCount(); ++row) {
        // Check if the next row will fit on the current page
        if (currentY + rowHeight > printer.pageRect(QPrinter::Point).height() - margin) {
            // Check if adding another row will exceed the page bottom and start a new page
            printer.newPage();
            currentY = margin;

            // Redraw headers on new page
            currentY = drawTableRow(headers, headerFont, true);
        }

        // Prepare row data
        QStringList rowData;
        for (int col = 0; col < model->columnCount(); ++col) {
            rowData << model->data(model->index(row, col)).toString();
        }

        // Draw the row
        currentY = drawTableRow(rowData, contentFont);
    }

    // Draw final horizontal line
    painter.drawLine(margin, currentY, margin + pageWidth, currentY);

    painter.end();

    QMessageBox::information(this, "Success", "PDF has been generated successfully!");
}


void clientwindow::updateStarRating(int rating) {
    currentRating = rating;

    // Update star appearance
    for (int i = 0; i < starButtons.size(); i++) {
        if (i < rating) {
            starButtons[i]->setText("★");
            starButtons[i]->setStyleSheet(
                "QPushButton { font-size: 24px; border: none; color: gold; }");
        } else {
            starButtons[i]->setText("☆");
            starButtons[i]->setStyleSheet(
                "QPushButton { font-size: 24px; border: none; } "
                "QPushButton:hover { color: gold; }");
        }
    }
}

void clientwindow::on_submitReviewButton_clicked() {
    if (currentRating == 0) {
        QMessageBox::warning(this, "Rating Required", "Please select a rating before submitting.");
        return;
    }

    QTextEdit* descriptionEdit = findChild<QTextEdit*>("descriptionEdit");
    if (!descriptionEdit) return;

    QString description = descriptionEdit->toPlainText().trimmed();
    if (description.isEmpty()) {
        QMessageBox::warning(this, "Description Required", "Please enter a review description.");
        return;
    }

    // Insert into database
    QSqlQuery query;
    query.prepare("INSERT INTO review (score, DESCRIPTION) VALUES (:score, :description)");
    query.bindValue(":score", currentRating);
    query.bindValue(":description", description);

    if (query.exec()) {
        QMessageBox::information(this, "Success", "Review submitted successfully!");
        // Reset form
        updateStarRating(0);
        descriptionEdit->clear();
    } else {
        QMessageBox::critical(this, "Error", "Failed to submit review: " + query.lastError().text());
    }
}
