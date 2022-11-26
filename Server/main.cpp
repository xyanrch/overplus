#include "Server/Service.h"
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
    auto& config = ConfigManage::instance();
    config.load_config(argv[1], ConfigManage::Server);
    std::unique_ptr<LogFile>logfile_;

    logger::set_log_level(config.server_cfg.log_level);
    if (!config.server_cfg.log_dir.empty()) {
        logfile_.reset(new LogFile("server", 10 * 1024 * 1024));
        logger::set_log_destination(Destination::D_FILE);
        logger::setOutput([&](std::string&& buf) {
            logfile_->append(std::move(buf));
        });
        logger::setFlush([&]() {
            logfile_->flush();
        });
    }
    try {
        Service server;
        server.run();
    } catch (const std::exception& e) {
        ERROR_LOG << "Server got exception,will exit :" << e.what();
    }
    NOTICE_LOG << "Server Stopped,will exit";

    return 0;
}
