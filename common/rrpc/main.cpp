
io_service io;
tcp_socket soc(io);
connection conn(soc);

auto p = make_shared<packet>(...);
conn.async_write(std::move(p), [](error_code ec){
});
