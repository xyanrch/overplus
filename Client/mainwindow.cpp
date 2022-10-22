#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include<QMessageBox>

MainWindow::MainWindow(Server&s,QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    ,server(s)
{
   // this->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(onConnect()));
    connect(ui->pushButton_2, SIGNAL(clicked()), this, SLOT(onDisconnect()));


}

MainWindow::~MainWindow()
{
    std::cout<<"destruct called mainwindow"<<std::endl;
   //server.stop();
    delete ui;

}
 void MainWindow::onConnect()
 {
     server.start_accept();
    // QMessageBox::about(this,"Tsignal","button clicked down");
     ui->pushButton->setEnabled(false);
      ui->pushButton_2->setEnabled(true);

 }

 void MainWindow::onDisconnect()
 {
     server.stop_accept();
     //QMessageBox::about(this,"Tsignal","button disconnect clicked down");
      ui->pushButton->setEnabled(true);
      ui->pushButton_2->setEnabled(false);
 }


