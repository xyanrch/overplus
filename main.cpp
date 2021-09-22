#include "SslServer.h"
//#include<string.h>
#include <string>

int main()
{
    ConfigManage::instance().load_config("server.json", ConfigManage::Server);
    SslServer server;
    server.run();

    return 0;
}