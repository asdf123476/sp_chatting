#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTcpSocket>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete socket;
}
void MainWindow::connect_server()
{
}

void MainWindow::ready_to_read()
{
    while (socket->canReadLine())
    {
        QString line = QString::fromUtf8(socket->readLine());
        line = line.left(line.length() - 1);
        ui->listWidget->addItem(QString("%2").arg(line));
    }
    ui->listWidget->scrollToBottom();
}

void MainWindow::on_msg_btn_clicked()
{

    QString msg = ui->msg_lineEdit->text().trimmed();
    QString name = ui->name_lineEdit->text().toUtf8();

    ui->listWidget->addItem(name + " : " + ui->msg_lineEdit->text());

    socket->write(QString(name + " : " + msg + "\n").toUtf8());
    ui->listWidget->addItem(QString(socket->readLine(50)));

    ui->msg_lineEdit->clear();
    ui->msg_lineEdit->setFocus();
}

void MainWindow::on_resigeter_btn_clicked()
{
    socket = new QTcpSocket(this);

    connect(socket, SIGNAL(connected()), this, SLOT(connect_server()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(ready_to_read()));

    ui->listWidget->addItem(QString("%2").arg(QString("Connecting..")).toUtf8());

    socket->connectToHost("127.0.0.1", 1025);

    if (!socket->waitForConnected(5000))
    {
        ui->listWidget->addItem(QString("%2").arg(QString("Connect Error!")));
    }

    // socket->write(QString("127.0.0.1").toUtf8());
}
