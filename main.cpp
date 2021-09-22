#include "SslServer.h"
//#include<string.h>
#include <string>

int main()
{
    SslServer server("server.json");
    server.run();

    return 0;
}