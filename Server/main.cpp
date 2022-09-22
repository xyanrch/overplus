#include "Server/SslServer.h"
#include "Shared/ConfigManage.h"
//#include<string.h>
#include "Shared/Log.h"
#include "Shared/LogFile.h"
#include <exception>
#include <string>

int main(int argc, char* argv[])
{

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << "config path" << std::endl;
        return 0;
    }
    ConfigManage::instance().load_config(argv[1], ConfigManage::Server);
    LogFile logfile_("server", 10 * 1024 * 1024);
    logger::set_log_level(ConfigManage::instance().server_cfg.log_level);
    logger::set_log_destination(Destination::D_FILE);
    logger::setOutput([&](std::string&& buf) {
        logfile_.append(std::move(buf));
    });
    logger::setFlush([&]() {
        logfile_.flush();
    });
    try {
        SslServer server;
        server.run();
    }
    catch (const std::exception& e) {
        ERROR_LOG << "Server got exception,will exit :" << e.what();
    }
    NOTICE_LOG << "Server Stopped,will exit";

    return 0;
}
