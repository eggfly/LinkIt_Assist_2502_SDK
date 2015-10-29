This example gets the time data from NTP server by UDP.

It creates a udp handle by vm_udp_create() to receive the udp messages, then calls vm_udp_send() to send a packet to a NTP server. When NTP server responsed it will call vm_udp_receive() to read the data.

You can change the NTP server address by modify g_address. 
Before run this example, please set the APN information first by modify macros.