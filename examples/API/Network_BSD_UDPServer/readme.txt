This sample creates a non-blocking UDP socket, after receiving a request, a response string is sent back.

It open the bearer by vm_bearer_open() and after the bearer is opened, it will create a sub thread by vm_thread_create(). In the sub thread, it will create a socket by API socket() and bind it to port 40002 by bind(). 

Once it is created it will check if there a request by select(), after a request is detected, it will receive the data by recv() and send the response by send();

You need to change the WLAN information according to the WLAN AP you used.