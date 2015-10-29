This example creates a non-blocking TCP server that listens to port 40000. When data is received it will send the RESPONSE_STRING to client. 

It opens the bearer by vm_bearer_open() and once the bearer is opened, it will create a sub thread by vm_thread_create(). Within the sub thread, it will bind the bearer to port 40000 by bind() and will start to listen for requests.

Once requests are arriving, it will receive the data by recv() and send the response by send();

You need to change the WLAN information according to your WLAN AP used.