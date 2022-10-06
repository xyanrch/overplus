#pragma once
#include "Log.h"
#include <boost/property_tree/ptree.hpp>
#include <unordered_set>

struct ServerConfig {
    //json format
    /*
    {
    "run_type": "server",
    "local_addr": "0.0.0.0",
    "local_port": 443,
    "allowed_password": [
        "password1",
        "password2"
    ],
    #NOTICE DEBUG ERROR
    "log_level": "NOTICE",
    "log_dir":"",
    "ssl": {
        "cert": "/path/to/certificate.crt",
        "key": "/path/to/private.key",
    }
    }
    */
    std::string local_addr;
    std::string local_port;
    std::unordered_set<std::string> allowed_passwords;
    //
    Loglevel log_level;
    std::string log_dir;
    std::string certificate_chain;
    std::string server_private_key;
    //
    void populate(boost::property_tree::ptree&);
};
struct ClientConfig {
    //todo
};
class ConfigManage : private boost::noncopyable {

public:
    enum ConfigType {
        Server,
        Client

    };

    void load_config(const std::string&, ConfigType);
    static ConfigManage& instance();

public:
    ClientConfig client_cfg;
    ServerConfig server_cfg;

private:
    ConfigManage() = default;
};