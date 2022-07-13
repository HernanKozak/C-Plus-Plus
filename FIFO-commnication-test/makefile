all: procesoServidor procesoCliente

procesoServidor: procesoServidor.o
	g++ -o  procesoServidor procesoServidor.o

procesoServidor.o: procesoServidor.cpp
	g++ -c procesoServidor.cpp

procesoCliente: procesoCliente.o
	g++ -o procesoCliente procesoCliente.o

procesoCliente.o: procesoCliente.cpp 
	g++ -c procesoCliente.cpp 
clean:
	rm *.o