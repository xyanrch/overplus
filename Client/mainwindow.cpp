#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include<QMessageBox>
#include "Shared/ConfigManage.h"

MainWindow::MainWindow(Server&s,QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    ,server(s)
{

    ui->setupUi(this);
    setWindowTitle("overplus");
    connect(ui->CONNECT_BUTTON, SIGNAL(clicked()), this, SLOT(onConnect()));
    connect(ui->DISCONNECT_BUTTON, SIGNAL(clicked()), this, SLOT(onDisconnect()));
    connect(ui->checkBox, SIGNAL(clicked()), this, SLOT(onCheckBoxClick()));

    if(ConfigManage::instance().loaded)
    {    auto& config = ConfigManage::instance().client_cfg;
         ui->HOST_NAME->setText(QString::fromStdString(config.remote_addr));
         ui->HOST_PORT->setText(QString::fromStdString(config.remote_port));
         ui->HOST_PASSWD->setText(QString::fromStdString(config.text_password));
    }


}

MainWindow::~MainWindow()
{

    delete ui;

}
 void MainWindow::onConnect()
 {

     ui->CONNECT_BUTTON->setEnabled(false);
     ui->DISCONNECT_BUTTON->setEnabled(true);
    ui->CONNECTION_STATUS->setText("CONNECTED");
   if(!ConfigManage::instance().loaded)
    {
       auto& config = ConfigManage::instance().client_cfg;
    config.remote_addr = ui->HOST_NAME->text().toStdString();
    config.remote_port = ui->HOST_PORT->text().toStdString();

    auto psswd = ui->HOST_PASSWD->text().toStdString();
    config.setPassword(psswd);
    NOTICE_LOG<<"Read config frome user input:"<<config.remote_addr<<":"<< config.remote_port<<" password:"<<psswd;
}
    //config.password = ui->
    server.start_accept();



 }

 void MainWindow::onDisconnect()
 {
     server.stop_accept();
      ui->CONNECT_BUTTON->setEnabled(true);
      ui->DISCONNECT_BUTTON->setEnabled(false);
      ui->CONNECTION_STATUS->setText("DISCONNECTED");

 }
 void MainWindow::onCheckBoxClick(){

      ui->HOST_PASSWD->setEchoMode(ui->checkBox->checkState() == Qt::Checked ? QLineEdit::Normal : QLineEdit::Password );
 }


