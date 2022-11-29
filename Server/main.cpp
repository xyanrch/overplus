#include "Server/Service.h"
#include "Shared/ConfigManage.h"
// #include<string.h>
#include "Shared/Log.h"
#include "Shared/LogFile.h"
#include "Shared/Version.h"
#include <boost/program_options.hpp>
#include <exception>
#include <string>
namespace po = boost::program_options;
int main(int argc, char* argv[])
{
    std::string config_file;
    po::options_description desc("Allowed options");
    desc.add_options()                     //
        ("version,v", "print version string") //
        ("help,h", "produce help message")    //
        ("config,c", po::value<std::string>(&config_file)->default_value("server.json"), "config file path");
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 0;
    }
    if (vm.count("version")) {
        std::cout << OVERPLUS_VERSION_STR << std::endl;
        return 0;
    }
    auto& config = ConfigManage::instance();
    config.load_config(config_file, ConfigManage::Server);
    std::unique_ptr<LogFile> logfile_;

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
