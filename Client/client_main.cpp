#include "Server.h"
#include "Shared/ConfigManage.h"
//#include<string.h>
#include <string>

int main()
{
    ConfigManage::instance().load_config("server.json", ConfigManage::Server);
    Server server("0.0.0.0", "1080");
    server.run();

    return 0;
}