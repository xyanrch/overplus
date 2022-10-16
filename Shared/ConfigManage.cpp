#include "ConfigManage.h"
#include <boost/property_tree/json_parser.hpp>
#include <cstdlib>
#include <openssl/evp.h>
#include <sstream>
#include <string>

//
static std::string SHA224(const std::string& message)
{
    uint8_t digest[EVP_MAX_MD_SIZE];
    char mdString[(EVP_MAX_MD_SIZE << 1) + 1];
    unsigned int digest_len;
    EVP_MD_CTX* ctx;
    if ((ctx = EVP_MD_CTX_new()) == nullptr) {
        throw std::runtime_error("could not create hash context");
    }
    if (!EVP_DigestInit_ex(ctx, EVP_sha224(), nullptr)) {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("could not initialize hash context");
    }
    if (!EVP_DigestUpdate(ctx, message.c_str(), message.length())) {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("could not update hash");
    }
    if (!EVP_DigestFinal_ex(ctx, digest, &digest_len)) {
        EVP_MD_CTX_free(ctx);
        throw std::runtime_error("could not output hash");
    }

    for (unsigned int i = 0; i < digest_len; ++i) {
        sprintf(mdString + (i << 1), "%02x", (unsigned int)digest[i]);
    }
    mdString[digest_len << 1] = '\0';
    EVP_MD_CTX_free(ctx);
    return std::string(mdString);
}

ConfigManage& ConfigManage::instance()
{
    static ConfigManage instance;
    return instance;
}
void ConfigManage::load_config(const std::string& path, ConfigType type)
{
    boost::property_tree::ptree tree;
    read_json(path, tree);

    if (type == ConfigType::Server) {
        server_cfg.populate(tree);
    }
    else if(type == ConfigType::Client)
    {
        client_cfg.populate(tree);

    }
    else {
        ERROR_LOG << "Expected server type config";
    }

}
void ClientConfig::populate(boost::property_tree::ptree& tree)
{
    local_addr = tree.get("local_addr", std::string());
    local_port = tree.get("local_port", std::string());
    remote_addr = tree.get("remote_addr", std::string());
    remote_port = tree.get("remote_port", std::string());
    user_name = tree.get("user_name", std::string());
    password = SHA224(tree.get("password", std::string()));
}
void ServerConfig::populate(boost::property_tree::ptree& tree)
{
    local_addr = tree.get("local_addr", std::string());
    local_port = tree.get("local_port", std::string());

    std::unordered_set<std::string>().swap(allowed_passwords);
    if (tree.get_child_optional("allowed_passwords")) {
        for (auto& item : tree.get_child("allowed_passwords")) {
            std::string p = item.second.get_value<std::string>();
            allowed_passwords.insert(SHA224(p));
        }
    }

    auto str_leve = tree.get("log_level", std::string());
    if (str_leve == "DEBUG") {
        log_level = L_DEBUG;
    } else if (str_leve == "NOTICE") {
        log_level = L_NOTICE;
    } else {
        log_level = L_ERROR_EXIT;
    }
    setLogLevel(log_level);
    log_dir = tree.get("log_dir", std::string());

    certificate_chain = tree.get("ssl.cert", std::string());
    server_private_key = tree.get("ssl.key", std::string());
}