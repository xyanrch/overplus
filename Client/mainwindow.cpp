#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include<QMessageBox>

MainWindow::MainWindow(Server&s,QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    ,server(s)
{

    ui->setupUi(this);
    connect(ui->CONNECT_BUTTON, SIGNAL(clicked()), this, SLOT(onConnect()));
    connect(ui->DISCONNECT_BUTTON, SIGNAL(clicked()), this, SLOT(onDisconnect()));


}

MainWindow::~MainWindow()
{

    delete ui;

}
 void MainWindow::onConnect()
 {
     server.start_accept();
     ui->CONNECT_BUTTON->setEnabled(false);
      ui->DISCONNECT_BUTTON->setEnabled(true);
   ui->CONNECTION_STATUS->setText("CONNECTED");

 }

 void MainWindow::onDisconnect()
 {
     server.stop_accept();
      ui->CONNECT_BUTTON->setEnabled(true);
      ui->DISCONNECT_BUTTON->setEnabled(false);
      ui->CONNECTION_STATUS->setText("DISCONNECTED");
 }


