#include "Server.h"
#include "Shared/ConfigManage.h"

#include <string>
#include "Shared/Log.h"
#include "Shared/LogFile.h"

#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    ConfigManage::instance().load_config("client.json", ConfigManage::Client);
    auto& config = ConfigManage::instance().client_cfg;
    LogFile logfile_("overplus", 10 * 1024 * 1024);
    logger::set_log_level(ConfigManage::instance().server_cfg.log_level);
    logger::set_log_destination(Destination::D_FILE);
    logger::setOutput([&](std::string&& buf) {
        logfile_.append(std::move(buf));
    });
    logger::setFlush([&]() {
        logfile_.flush();
    });
    Server server(config.local_addr, config.local_port);
    QApplication a(argc, argv);
    MainWindow w(server);
    w.show();
   std::shared_ptr<std::thread> backgroud(new std::thread([&server] {
        server.run();
       //std::cout<<"hello"<<std::endl;
    }));

    auto rc = a.exec();
    //server.stop();
    std::cout<<"before join"<<std::endl;
    //backgroud->join();
    std::cout<<"after join"<<std::endl;
  server.stop();
  backgroud->join();
  //backgroud->
    return rc;
}

