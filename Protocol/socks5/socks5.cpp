#include "socks5.h"

std::string method_to_string(AuthMethod method)
{
    switch (method) {
    case NO_AUTHENTICATION:
        return "NO_AUTHENTICATION";
    case GSSAPI:
        return "GSSAPI";
    case PASSWORD:
        return "PASSWORD";
    case IANA_ASSIGNED:
        return "IANA_ASSIGNED";
    case RESERVED:
        return "RESERVED"; // 0X80-0XFE
    case NO_ACCEPTABLE_METHODS:
        return "NO_ACCEPTABLE_METHODS";
    }
    return "";
}
std::ostream& operator<<(std::ostream& os, AuthReq& req)
{
    os << "AuthReq {VER:" << (int)req.version << ", methods: [";
    for (int i = 0; i < req.nmethod; i++) {
        os << method_to_string(req.methods[i]) << " ";
    }
    os << "]}";
    return os;
}

bool AuthReq::unstream(std::vector<char>& buf)
{

    char* ptr = buf.data();

    version = Coding::DecodeFixed8(ptr);

    ptr++;
    if (version != (char)0x05) {
        ERROR_LOG << "Invalid version number:" << version;
        return false;
    }
    nmethod = Coding::DecodeFixed8(ptr);
    ptr++;
    for (int i = 0; i < nmethod; ++i) {
        AuthMethod method;
        auto method_oct = Coding::DecodeFixed8(ptr);
        if (i < nmethod - 1) {
            ptr++;
        }
        if (method_oct <= 0x02) {
            method = static_cast<AuthMethod>(method_oct);
        } else if (method_oct <= 0x7e) {
            method = IANA_ASSIGNED;
        } else if (method_oct <= 0xfe) {
            method = RESERVED;
        } else {
            method = NO_ACCEPTABLE_METHODS;
        }
        methods.push_back(method);
    }
    return true;
}

bool Request::unsteam(std::vector<char>& buf, int length)
{

    char* ptr = buf.data();
    version = Coding::DecodeFixed8(ptr);
    if (version != (char)0x05) {
        ERROR_LOG << "Invalid version number:" << (int)version;
        return false;
    }
    ptr++;
    cmd = static_cast<CMD>(Coding::DecodeFixed8(ptr));
    ptr++;
    reserved = Coding::DecodeFixed8(ptr);
    if (reserved != (char)0x00) {
        ERROR_LOG << "Invalid reserved value:" << (int)reserved;
        return false;
    }
    ptr++;
    auto val = Coding::DecodeFixed8(ptr);
    if (val == 0x01) {
        addrType = V4;
    } else if (val == 0x03) {
        addrType = DOMAINNAME;
    } else if (val == 0x04) {
        addrType = V6;
    } else {
        ERROR_LOG << "Invalid address type:" << (int)val;
        return false;
    }
    ptr++;
    switch (addrType) {
    case V4: // IP V4 addres
        if (length != (size_t)(4 + 4 + 2)) {
            ERROR_LOG << "SOCKS5 request(V4) length is invalid. Closing session.";
            return false;
        }
        remote_host = boost::asio::ip::address_v4(ntohl(Coding::DecodeFixed32(ptr))).to_string();
        ptr = ptr + 4;
        remote_port = ntohs(Coding::DecodeFixed16(ptr));
        break;
    case DOMAINNAME: // DOMAINNAME
    {
        auto host_length = Coding::DecodeFixed8(ptr);
        ptr++;
        if (length != (size_t)(5 + host_length + 2)) {
            ERROR_LOG << "SOCKS5(domain name) request length is invalid. Closing session." << length;
            return false;
        }
        remote_host = std::string(ptr, host_length);
        ptr = ptr + host_length;
        remote_port = ntohs(Coding::DecodeFixed16(ptr));
    } break;
    default:
        ERROR_LOG << "unsupport_ed address type in SOCKS5 request. Closing session.";
        return false;
    }
    return true;
}
 std::string Request::command_to_string(CMD cmd)
{
    if (cmd == CONNECT)
        return "CONNECT";
    if (cmd == BIND)
        return "BIND";
    if (cmd == UDP_ASSOCIATE)
        return "UDP_ASSOCIATE";
    return "ERR";
}
std::ostream& operator<<(std::ostream& os, Request& req)
{
    os << "Request {VER:" << (int)req.version << ", command:" << Request::command_to_string(req.cmd)
       << ", remote_host: " << req.remote_host << ", remote_port:" << req.remote_port << " }";
    return os;
}