#include "WebsocketSession.h"
#include "Shared/ConfigManage.h"
#include "Shared/Log.h"
WebsocketSession::WebsocketSession(boost::asio::io_context& io_ctx, boost::asio::ssl::context& ssl_ctx)
: Session<websocket::stream<beast::ssl_stream<beast::tcp_stream>>>(io_ctx,ssl_ctx)
{
}
void WebsocketSession::start()
{
    // Set the timeout.
    beast::get_lowest_layer(upstream_socket).expires_after(std::chrono::seconds(30));

    // Perform the SSL handshake
    auto self = shared_from_this();
    upstream_socket.next_layer().async_handshake(
        ssl::stream_base::server,
        [this,self](beast::error_code ec){
            on_handshake(ec);
        });
}
void WebsocketSession::on_handshake(beast::error_code ec)
{
    if (ec) {
        ERROR_LOG << "SSL handshak failed:" << ec.message();
        return;
    }
    // Turn off the timeout on the tcp_stream, because
    // the websocket stream has its own timeout system.
    beast::get_lowest_layer(upstream_socket).expires_never();

    // Set suggested timeout settings for the websocket
    upstream_socket.set_option(
        websocket::stream_base::timeout::suggested(
            beast::role_type::server));

    // Set a decorator to change the Server of the handshake
    upstream_socket.set_option(websocket::stream_base::decorator(
        [](websocket::response_type& res) {
            res.set(http::field::server,"overplus websocket-server");
        }));

    // Accept the websocket handshake
    auto self = shared_from_this();
    upstream_socket.async_accept(
        [this,self](beast::error_code ec){
            on_accept(ec);
        }
  );
}
void WebsocketSession::on_accept(beast::error_code ec)
{
    if (ec) {
        ERROR_LOG << "websocket accept failed:" << ec.message();
        return;
    }
    handle_custom_protocol();
}
void WebsocketSession::upstream_tcp_write(int direction, size_t len)
{
    auto self(this->shared_from_this());
    upstream_socket.async_write(boost::asio::buffer(out_buf, len), [this, self, direction](boost::system::error_code ec, std::size_t length) {
        if (!ec)
            async_bidirectional_read(direction);
        else {
            if (ec != boost::asio::error::operation_aborted) {
                NOTICE_LOG << "Client<--Server(TCP):" <<ec.message();
            }
            destroy();
            return;
        }
    });
}

void WebsocketSession::upstream_udp_write(int direction, const std::string& packet)
{
    auto self(this->shared_from_this());
    upstream_socket.async_write(boost::asio::buffer(packet),
        [this, self, direction](boost::system::error_code ec, std::size_t length) {
            if (!ec)
                udp_async_bidirectional_read(direction);
            else {
                if (ec != boost::asio::error::operation_aborted) {
                    NOTICE_LOG << "Client<--Server(UDP over tls)" << ec.message();
                }
                destroy();
                return;
            }
        });
}
void WebsocketSession::destroy()
{
    if (state_ == DESTROY) {
        return;
    }
    state_ = DESTROY;
    Session<websocket::stream<beast::ssl_stream<beast::tcp_stream>>>::destroy();
}
