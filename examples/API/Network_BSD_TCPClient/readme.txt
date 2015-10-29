This example creates a TCP Client and sends an http request to a server and prints out the first 20 bytes of the data that is  received from server.

It opens the bearer by vm_bearer_open() and after the bearer is opened creates a sub thread by vm_thread_create(). In the sub thread, it establishes a connection to CONNECT_ADDRESS. After the connection is established it sends the string by send() and receives the response by recv(); 

You can change the connect address by modify MACRO CONNECT_ADDRESS, change the APN information according to your SIM card.
