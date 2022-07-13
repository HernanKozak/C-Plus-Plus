#include <stdio.h>
#include <string>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <dirent.h>
#include <errno.h>
#include <signal.h>


using std::string;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::fstream;
using std::vector;
using std::ios;

void INThandler(int signum)
{
    remove("/tmp/fifoEj03");
    exit(0);
}

string getComando(string data)
{
    return data.substr(0, data.find(" "));
}

string getParam(string data)
{
    return data.substr(data.find(" ") + 1);
}

string getRuta(string linea){
    return linea.substr(0, linea.find_last_of("/"));
}

string recieve(){
    string data;
    mkfifo("/tmp/fifoEj03", 0666);
    ifstream fifo("/tmp/fifoEj03");

    getline(fifo, data);

    fifo.close();
    return data;
}

void send(vector<string> msj){
    mkfifo("/tmp/fifoEj03", 0666);
    ofstream fifo("/tmp/fifoEj03");

    for (string item : msj)
    {
        fifo << item << endl;
    }
    fifo.close();
}

void sendError(string error){

    mkfifo("/tmp/fifoEj03", 0666);
    ofstream fifo("/tmp/fifoEj03");
    fifo << error;
    fifo.close();
}

bool exists_test (const char* arch) {
    return ( access( arch, F_OK ) != -1 );
}

vector<string> readFile(const char* archivoL){
    vector<string> lineas;
    fstream file;
    file.open(archivoL, ios::in);
    string linea;
    if ( file.is_open() ) {
        while ( !file.eof() ) {
            getline (file, linea);
            lineas.push_back (linea);
        }
    }
    return lineas;
}

string mkfile (string data, vector<string> directorios){
    FILE *fp;
    string parametro=getParam(data);
    string ruta = getRuta(parametro);
    if (std::find(directorios.begin(), directorios.end(), ruta) == directorios.end()){
        return("La ruta indicada es incorrecta o no se encuentra entre las habilitadas");
    }else{
        DIR* dir = opendir(ruta.c_str());
        if (dir) {
            fp  = fopen (parametro.c_str(), "w");
            fclose(fp);
            closedir(dir);
        } else if (ENOENT == errno) {
            return ("el directorio indicado no existe");
        } else {
            return("no fue posible acceder al directorio especificado ");
        }
    }
    return ("archivo creado con exito");
}

vector<string> rdfile (string data, vector<string> directorios){
    string archivo = getParam(data);
    string ruta = getRuta(archivo);
    vector<string> resultado;

    if (std::find(directorios.begin(), directorios.end(), ruta) == directorios.end()){
        resultado.push_back("La ruta indicada es incorrecta o no se encuentra entre las habilitadas");
    }
    else if (!exists_test(archivo.c_str())){
        resultado.push_back("El archivo ingresado no es correcto o no es posible acceder a el");
    }
    else{
        resultado=readFile(archivo.c_str());
    }
    return resultado;
}

string delfile (string data, vector<string> directorios){
    FILE *fp;
    string archivo=getParam(data);
    string ruta = getRuta(archivo);
    if (std::find(directorios.begin(), directorios.end(), ruta) == directorios.end()){
        return("La ruta indicada es incorrecta o no se encuentra entre las habilitadas");
    }
    if (!exists_test(archivo.c_str())){
        return("El archivo ingresado no es correcto o no es posible acceder a el");
    }
    if (remove(archivo.c_str()) != 0) {
        return ("El archivo no pudo ser eliminado");
    }

    return ("archivo eliminado con exito");
}

void ayuda(){
    cout << endl;
    cout << "Este es el proceso Servidor" << endl;
    cout << "Recibe parámetros y los ejecuta. Los parametros validos son: " << endl;
    cout << "lstdir = lista los directorios válidos" << endl;
    cout << "mkfile [Parametro]= crea el archivo recibido por parametro" << endl;
    cout << "rdfile [Parametro]= lee el archivo recibido por parametro" << endl;
    cout << "delfile [Parametro]= elimina el archivo recibido por parametro" << endl;
    cout << "srvstp = detiene el servidor" << endl << endl;
    cout << "Ejemplos de ejecución del servidor: " << endl;
    cout << "./procesoServidor dir" << endl;
    cout << "./procesoServidor direcciones" << endl << endl;
}



int main(int argc, char *arg[])
{
    if(argc!=2){
        cout << "Cantidad incorrecta de parámetros" << endl;
        return 1;
    }

    signal(SIGINT, INThandler);
    signal(SIGTERM, INThandler);


    string comando = getComando(arg[1]);

    if(comando == "-h" || comando == "--help"){
        ayuda();
        return 0;
    }

    if(!exists_test(arg[1])){
        cout << "El archivo ingresado no es correcto o no es posible acceder a el" << endl;
        return 1;
    }

    if (fork() != 0){
        exit(0);
    }
    setsid();


    vector<string> directorios = readFile(arg[1]);
    vector<string> respuesta;

    string data, rsp;

    cout << "el fifo se encuentra en: /tmp/fifoEj03" << endl;

    while (1)
    {
        bool flagParametro = true;
        data=recieve();
        comando = getComando(data);
        if(comando == "lstdir"){
            send(directorios);
            flagParametro=false;
        }
        if(comando == "mkfile"){
            rsp=mkfile(data, directorios);
            respuesta.push_back(rsp);
            send(respuesta);
            flagParametro=false;
        }
        if(comando == "rdfile"){
            respuesta=rdfile(data, directorios);
            send(respuesta);
            flagParametro=false;
        }

        if(comando == "delfile"){
            rsp=delfile(data,directorios);
            respuesta.push_back(rsp);
            send(respuesta);
            flagParametro=false;
        }

        if(comando == "srvstp"){
            respuesta.push_back("servidor detenido con exito");
            send(respuesta);
            break;
        }

        if(flagParametro){
            sendError("El comando ingresado no es correcto");
        }

        respuesta.clear();
    }
    remove("/tmp/fifoEj03");
    return 0;
}
