#include "Server.h"
#include "Shared/ConfigManage.h"
//#include<string.h>
#include <string>

int main()
{
    ConfigManage::instance().load_config("client.json", ConfigManage::Client);
    auto& config = ConfigManage::instance().client_cfg;
    Server server(config.local_addr, config.local_port);
    server.run();

    return 0;
}