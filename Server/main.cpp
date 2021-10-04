#include "Server/SslServer.h"
#include "Shared/ConfigManage.h"
//#include<string.h>
#include "Shared/Log.h"
#include "Shared/LogFile.h"
#include <string>

int main()
{
    ConfigManage::instance().load_config("server.json", ConfigManage::Server);
    LogFile logfile_("server", 10 * 1024 * 1024);
    logger::set_log_level(ConfigManage::instance().server_cfg.log_level);
     logger::set_log_destination(Destination::D_FILE);
    logger::setOutput([&](std::string&& buf) {
        logfile_.append(std::move(buf));
    });
    logger::setFlush([&]() {
        logfile_.flush();
    });

    SslServer server;
    server.run();

    return 0;
}
