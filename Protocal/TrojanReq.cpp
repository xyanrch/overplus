#include "TrojanReq.h"
#include "Shared/Log.h"
using namespace std;
bool SOCKS5Address::parse(const string& data, size_t& address_len)
{
    if (data.length() == 0 || (data[0] != IPv4 && data[0] != DOMAINNAME && data[0] != IPv6)) {
        return false;
    }
    address_type = static_cast<AddressType>(data[0]);
    switch (address_type) {
    case IPv4: {
        if (data.length() > 4 + 2) {
            address = to_string(uint8_t(data[1])) + '.' + to_string(uint8_t(data[2])) + '.' + to_string(uint8_t(data[3])) + '.' + to_string(uint8_t(data[4]));
            port = (uint8_t(data[5]) << 8) | uint8_t(data[6]);
            address_len = 1 + 4 + 2;
            return true;
        }
        break;
    }
    case DOMAINNAME: {
        uint8_t domain_len = data[1];
        if (domain_len == 0) {
            // invalid domain len
            break;
        }
        if (data.length() > (unsigned int)(1 + domain_len + 2)) {
            address = data.substr(2, domain_len);
            port = (uint8_t(data[domain_len + 2]) << 8) | uint8_t(data[domain_len + 3]);
            address_len = 1 + 1 + domain_len + 2;
            return true;
        }
        break;
    }
    case IPv6: {
        if (data.length() > 16 + 2) {
            char t[40];
            sprintf(t, "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
                uint8_t(data[1]), uint8_t(data[2]), uint8_t(data[3]), uint8_t(data[4]),
                uint8_t(data[5]), uint8_t(data[6]), uint8_t(data[7]), uint8_t(data[8]),
                uint8_t(data[9]), uint8_t(data[10]), uint8_t(data[11]), uint8_t(data[12]),
                uint8_t(data[13]), uint8_t(data[14]), uint8_t(data[15]), uint8_t(data[16]));
            address = t;
            port = (uint8_t(data[17]) << 8) | uint8_t(data[18]);
            address_len = 1 + 16 + 2;
            return true;
        }
        break;
    }
    }
    return false;
}

string SOCKS5Address::generate(const boost::asio::ip::udp::endpoint& endpoint)
{
    if (endpoint.address().is_unspecified()) {
        return string("\x01\x00\x00\x00\x00\x00\x00", 7);
    }
    string ret;
    if (endpoint.address().is_v4()) {
        ret += '\x01';
        auto ip = endpoint.address().to_v4().to_bytes();
        for (int i = 0; i < 4; ++i) {
            ret += char(ip[i]);
        }
    }
    if (endpoint.address().is_v6()) {
        ret += '\x04';
        auto ip = endpoint.address().to_v6().to_bytes();
        for (int i = 0; i < 16; ++i) {
            ret += char(ip[i]);
        }
    }
    ret += char(uint8_t(endpoint.port() >> 8));
    ret += char(uint8_t(endpoint.port() & 0xFF));
    return ret;
}
int TrojanReq::parse(const string_view& data)
{
    size_t first = data.find("\r\n");
    if (first == string::npos) {
        return -1;
    }
    password = data.substr(0, first);
    payload = data.substr(first + 2);
    if (payload.length() == 0 || (payload[0] != CONNECT && payload[0] != UDP_ASSOCIATE)) {
        return -1;
    }
    command = static_cast<Command>(payload[0]);
    size_t address_len;
    bool is_addr_valid = address.parse(payload.substr(1), address_len);
    if (!is_addr_valid || payload.length() < address_len + 3 || payload.substr(address_len + 1, 2) != "\r\n") {
        return -1;
    }
    payload = payload.substr(address_len + 3);
    return data.length();
}

string TrojanReq::generate(const string& password, const string& domainname, uint16_t port, bool tcp)
{
    string ret = password + "\r\n";
    if (tcp) {
        ret += '\x01';
    } else {
        ret += '\x03';
    }
    ret += '\x03';
    ret += char(uint8_t(domainname.length()));
    ret += domainname;
    ret += char(uint8_t(port >> 8));
    ret += char(uint8_t(port & 0xFF));
    ret += "\r\n";
    return ret;
}