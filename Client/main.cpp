
#include "Server.h"
#include "Shared/ConfigManage.h"

#include <string>
#include "Shared/Log.h"
#include "Shared/LogFile.h"

#include "mainwindow.h"

#include <QApplication>
#include<windows.h>
#include<stdio.h>

bool  enable_system_socks_proxy()
{
    HKEY hRoot = HKEY_CURRENT_USER;
    const char* szSubKey="Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings";
    HKEY hKey;
    DWORD dwDisposition = REG_OPENED_EXISTING_KEY;
    LONG lRet = RegCreateKeyEx(hRoot, szSubKey, 0, NULL,
            REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition);
     if (lRet != ERROR_SUCCESS)
            return false;

     //enable proxy
     int val = 1;

     lRet = RegSetValueEx(hKey, "ProxyEnable", 0, REG_DWORD, (BYTE*)&val, sizeof(DWORD));

     if (lRet == ERROR_SUCCESS)
     {
         printf("enable proxy succussfully!\n");
     }
     else
     {
         printf("enable proxy failed!\n");
         return false;
     }

     RegCloseKey(hKey);
     return true;

}

bool  disable_system_socks_proxy()
{
    HKEY hRoot = HKEY_CURRENT_USER;
    const char* szSubKey="Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings";
    HKEY hKey;
    DWORD dwDisposition = REG_OPENED_EXISTING_KEY;
    LONG lRet = RegCreateKeyEx(hRoot, szSubKey, 0, NULL,
            REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition);
     if (lRet != ERROR_SUCCESS)
            return false;
     const char*proxy_protocol="socks://127.0.0.1:1080";
     lRet = RegSetValueEx(hKey, "ProxyServer", 0, REG_SZ, (BYTE*)proxy_protocol, strlen(proxy_protocol));

     if (lRet == ERROR_SUCCESS)
     {
         printf("proxy protocol set succussfully!\n");
     }
     else
     {
         printf("proxy protocol set failed!\n");
         return false;
     }

     //enable proxy
     int val = 0;

     lRet = RegSetValueEx(hKey, "ProxyEnable", 0, REG_DWORD, (BYTE*)&val, sizeof(DWORD));

     if (lRet == ERROR_SUCCESS)
     {
         printf("Disable proxy succussfully!\n");
     }
     else
     {
         printf("Diable proxy failed!\n");
         return false;
     }

     RegCloseKey(hKey);
     return true;

}
int main(int argc, char *argv[])
{   ConfigManage::instance().load_config("client.json", ConfigManage::Client);
    auto& config = ConfigManage::instance().client_cfg;
    LogFile logfile_("overplus", 10 * 1024 * 1024);
    //logger::set_log_level(ConfigManage::instance().server_cfg.log_level);
    logger::set_log_destination(Destination::D_FILE);
    logger::setOutput([&](std::string&& buf) {
        logfile_.append(std::move(buf));
    });
    logger::setFlush([&]() {
        logfile_.flush();
    });
    if(!enable_system_socks_proxy())
    {
        return 1;
    }
    Server server(config.local_addr, config.local_port);
    QApplication a(argc, argv);
    MainWindow w(server);
    w.show();
   std::shared_ptr<std::thread> backgroud(new std::thread([&server] {
        server.run();
    }));

    auto rc = a.exec();
  server.stop();
  backgroud->join();

  disable_system_socks_proxy();

    return rc;
}

