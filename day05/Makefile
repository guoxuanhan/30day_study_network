server:
	g++ util.cpp server.cpp Epoll.cpp InetAddress.cpp Socket.cpp Channel.cpp -o server && \
	g++ util.cpp client.cpp -o client
clean:
	rm server && rm client