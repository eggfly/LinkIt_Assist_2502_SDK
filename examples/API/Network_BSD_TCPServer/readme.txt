This example creates TCP blocking server listen the port 40000, after receiving the data, send the RESPONSE_STRING to client.

It opens the WiFi bearer by vm_bearer_open(). Once the bearer is opened, it will create a sub thread by vm_thread_create(). In the sub thread, it will bind itself to port 40000 by bind() and start listening to requests. When a request is received, it will receive the data by recv() and send the response by send();

You need to change the WiFi SSID and PASSWORD by modify the macro AP_SSID and AP_PASSWORD.

You can see the local ip in log, if send a request to the port 40000 of this ip, it will print the first 20 bytes of the request data out to log.
