#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
     ui(new Ui::MainWindow),
     m_tileserver(new TileserverManager(this))
{
    ui->setupUi(this);
    // Connect TileserverManager signals for logs
    connect(m_tileserver, &TileserverManager::outputReceived, [](const QString &output) {
        qDebug() << "[Tileserver Log]:" << output;
    });

    connect(m_tileserver, &TileserverManager::errorReceived, [](const QString &error) {
        qWarning() << "[Tileserver Error]:" << error;
    });

}

MainWindow::~MainWindow()
{
    delete ui;
    m_tileserver->stopTileserver(8082);
}




void MainWindow::on_startButton_clicked()
{
    QString dataPath = "/mnt/d/icdas/tileserver/data";
    int port =8082;
    m_tileserver->startTileserver(dataPath,port);
}


void MainWindow::on_stopButton_clicked()
{
    int port =8082;
     m_tileserver->stopTileserver(port);
}

