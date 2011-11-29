all: echo tcp udp udp_client
	gcc -Wall -o myinetd inetd.c
  
echo:
	gcc -Wall -o server_echo server_echo.c
  
tcp:
	gcc -Wall -o server_time_tcp server_time_TCP.c
  
udp:
	gcc -Wall -o server_time_udp server_time_UDP.c
  
udp_client:
	gcc -Wall -o udp_client cl_UDP_teste.c
