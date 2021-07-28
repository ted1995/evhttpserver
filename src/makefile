server: main.cpp ./handler.cpp ./sqlpool.cpp ./webserver.cpp ./workers.cpp 
	g++ -g -std=c++11 -o http-server main.cpp ./handler.cpp ./sqlpool.cpp ./webserver.cpp ./workers.cpp -lmysqlclient -levent -lglog -lpthread

clean:
	rm  -r server
