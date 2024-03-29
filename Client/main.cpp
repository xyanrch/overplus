
#include "Server.h"
#include "Shared/ConfigManage.h"

#include "Shared/Log.h"
#include "Shared/LogFile.h"
#include <string>

#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <stdio.h>
#include <windows.h>
const char* LOCAL_PROXY_ADDR = "127.0.0.1";
const char* LOCAL_PROXY_PORT = "1080";
const char* LOCAL_PROXY_PROTOCOL = "socks://127.0.0.1:1080";
bool disable_system_socks_proxy()
{
    HKEY hRoot = HKEY_CURRENT_USER;
    const char* szSubKey = "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings";
    HKEY hKey;
    DWORD dwDisposition = REG_OPENED_EXISTING_KEY;
    LONG lRet = RegCreateKeyEx(hRoot, szSubKey, 0, NULL,
        REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition);
    if (lRet != ERROR_SUCCESS)
        return false;

    // disable proxy
    int val = 0;

    lRet = RegSetValueEx(hKey, "ProxyEnable", 0, REG_DWORD, (BYTE*)&val, sizeof(DWORD));

    if (lRet == ERROR_SUCCESS) {
        NOTICE_LOG << "disable proxy succussfully!";
    } else {
        ERROR_LOG << "disable proxy failed!";
        return false;
    }

    RegCloseKey(hKey);
    return true;
}

bool enable_system_socks_proxy()
{
    HKEY hRoot = HKEY_CURRENT_USER;
    const char* szSubKey = "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings";
    HKEY hKey;
    DWORD dwDisposition = REG_OPENED_EXISTING_KEY;
    LONG lRet = RegCreateKeyEx(hRoot, szSubKey, 0, NULL,
        REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition);
    if (lRet != ERROR_SUCCESS)
        return false;
    lRet = RegSetValueEx(hKey, "ProxyServer", 0, REG_SZ, (BYTE*)LOCAL_PROXY_PROTOCOL, strlen(LOCAL_PROXY_PROTOCOL));

    if (lRet == ERROR_SUCCESS) {
        NOTICE_LOG << "proxy protocol set succussfull";
    } else {
        ERROR_LOG << "proxy protocol set failed!";
        return false;
    }

    // enable proxy
    int val = 1;

    lRet = RegSetValueEx(hKey, "ProxyEnable", 0, REG_DWORD, (BYTE*)&val, sizeof(DWORD));

    if (lRet == ERROR_SUCCESS) {
        NOTICE_LOG << "enable proxy succussfully";
    } else {
        ERROR_LOG << "enable proxy failed!";
        return false;
    }

    RegCloseKey(hKey);
    return true;
}
int main(int argc, char* argv[])
{
    QFileInfo file("client.json");
    auto& config = ConfigManage::instance().client_cfg;
    if (file.isFile()) {
        ConfigManage::instance().load_config("client.json", ConfigManage::Client);

    } else {
        config.local_addr = LOCAL_PROXY_ADDR;
        config.local_port = LOCAL_PROXY_PORT;
        config.user_name = "test_usr";
    }
    LogFile logfile_("overplus", 10 * 1024 * 1024);
    // logger::set_log_level(ConfigManage::instance().server_cfg.log_level);
    logger::set_log_destination(Destination::D_FILE);
    logger::setOutput([&](std::string&& buf) {
        logfile_.append(std::move(buf));
    });
    logger::setFlush([&]() {
        logfile_.flush();
    });
    if (!enable_system_socks_proxy()) {
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
