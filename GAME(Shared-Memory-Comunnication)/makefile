all: cliente servidor

servidor: servidor.o
	g++ -o  servidor servidor.o -lpthread

servidor.o: servidor.cpp
	g++ -c servidor.cpp -lpthread

cliente: cliente.o
	g++ -o cliente cliente.o -lpthread
	
cliente.o: cliente.cpp 
	g++ -c cliente.cpp -lpthread
clean:
	rm *.o
