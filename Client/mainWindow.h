#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include"Server.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(Server&s,QWidget *parent = nullptr);
    ~MainWindow();
public slots:
    void onConnect();
    void onDisconnect();
private:
    Ui::MainWindow *ui;
    Server& server;
};
#endif // MAINWINDOW_H
