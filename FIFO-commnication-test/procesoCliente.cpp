#include <stdio.h>
#include <string>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <fstream>


using std::string;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;

void sendArg(const char *arg){
    mkfifo("/tmp/fifoEj03", 0666);
    ofstream fifo("/tmp/fifoEj03");
    fifo << arg;
    fifo.close();
}

void ayuda(){
    cout << endl;
    cout << "Este es el proceso Cliente" << endl;
    cout << "Envía parámetros y el servidor los ejecuta. Los parametros validos son: " << endl;
    cout << "lstdir = lista los directorios válidos" << endl;
    cout << "mkfile [Parametro]= crea el archivo recibido por parametro" << endl;
    cout << "rdfile [Parametro]= lee el archivo recibido por parametro" << endl;
    cout << "delfile [Parametro]= elimina el archivo recibido por parametro" << endl;
    cout << "srvstp = detiene el servidor" << endl << endl;
    cout << "Ejemplos de ejecución: " << endl;
    cout << "./procesoCliente lstdir" << endl;
    cout << "./procesoCliente mkfile /home/hernan/dir1/hernan.txt" << endl;
    cout << "./procesoCliente rdfile /home/hernan/dir1/hernan.txt" << endl;
    cout << "./procesoCliente delfile /home/hernan/dir1/hernan.txt" << endl;
    cout << "./procesoCliente srvstp" << endl << endl << endl;
}


int main(int argc, char *arg[])
{
    if(argc!=2 && argc!=3){
        printf("Cantidad incorrecta de parámetros");
        exit(1);
    }
    string comando = arg[1];

    if(comando == "-h" || comando == "--help"){
        ayuda();
        return 0;
    }

    comando.append(" ");

    if(argc == 3){
        comando.append(arg[2]);
    }

    sendArg(comando.c_str());

    string respuesta;

    ifstream fifo("/tmp/fifoEj03");
    while (!fifo.eof()){
        getline(fifo, respuesta);
        cout << respuesta << endl;
    }

    fifo.close();
    return 0;
}
