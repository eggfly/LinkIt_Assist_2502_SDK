This example gets the local address and peer address in network byte order. 

It opens the bearer by vm_bearer_open(), after the bearer is opened, it will create a sub thread by vm_thread_create(). Within the sub thread, it will establish a connection to CONNECT_ADDRESS and after the connection is established, it will get the local and peer address by getsockname() and getpeername(). 

You can change the connect address by modify MACRO CONNECT_ADDRESS. Change the APN information according to your SIM card. You can see the local address and peer address by searching keywords "local address" and "peer address" in the log about 2 minutes after bootup.
