This sample executes an HTTPS request and prints the content of the request to the log.

It connects the port 443 of the host by API vm_ssl_connect(), a callback function is registered with this API. When the VM_SSL_EVENT_CAN_WRITE event is received, it will call vm_ssl_write() to send a HTTP request to get the URL. When the VM_SSL_EVENT_CAN_READ even is received, it will call vm_ssl_read() to read the content.

Before running this application, you need to put the certificate of the certification authority of the host you want to connect to into the CERTIFICATE_PATH. You can change the URL, host and certificate path by modify the corresponding MACROs.
Please set the APN information according to your SIM card.