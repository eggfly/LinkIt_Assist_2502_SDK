This example creates a TCP Client in synchronous way and sends a HTTP request to an address and reads the response.

It creates a sub-thread by using vm_thread_create(). In the sub-thread, it connects a server by calling vm_tcp_connect_sync(). After the connection is established, it calls vm_tcp_write_sync() to send a request and calls vm_tcp_read_sync() to read the response.

The connection address can be changed by modifying the macro HTTP_HOST and HTTP_URL. The cellular APN should be set according to the SIM card information.
This example requires a valid SIM card installed on the board.
