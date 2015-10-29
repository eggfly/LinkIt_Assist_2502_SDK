This example creates TCP Client and sends a HTTP request to an server and prints the first 20 bytes of the data received to the log on the Monitor.

It connects the server by vm_tcp_connect(). After the connection is established, it will call vm_tcp_write() to send the request. When the response arrives, it calls vm_tcp_read() to read the data from the response.

The connection address can be changed by modifying the macro CONNECT. The APN should be set according to the SIM card information.
  
This example requires a valid SIM card installed on the board.
