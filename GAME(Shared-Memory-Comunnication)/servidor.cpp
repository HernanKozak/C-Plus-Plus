#include "./bibliotecas.hpp"

static partida *datoPartida;
static int shmid;
static int cantidadJugadores;
static sem_t* semaforos[3];

void INThandler(int);
void ctrlHandler(int);
string obtenerPalabraDeArchivo();
bool buscarGuion(char*);
int buscarGanador(int*, int);
void cerrarEliminarSem();
void liberarSemaforos();
bool mostrarAyuda(const char*);
void inicializarSemaforos();
bool esLetraPalabra(string, char);

int main(int argc, char *argv[])
{
    signal(SIGINT, ctrlHandler);
    signal(SIGTERM, INThandler);

    if(argc > 2)
    {
        cerr << "Error: Cantidad de parametros invalida" << endl;
        exit(1);
    }
    if(argc == 2)
    {
        if(!mostrarAyuda(argv[1]))
            cerr << "Error: parametro invalidos" << endl;
        exit(1);
    }

    if (fork() != 0){
        exit(0);
    }
    setsid();


    int jugadores[3] ={0,0,0};

    // ftok to generate unique key
    key_t key = ftok("shmfile",65);

    // shmget returns an identifier in shmid
    shmid = shmget(key,1024,0666|IPC_CREAT);
    string palabra =obtenerPalabraDeArchivo();
    int cantLetrasPalabra = palabra.length();
    string letrasIngresadas;

    // shmat to attach to shared memory
    datoPartida = (partida*) shmat(shmid,(void*)0,0);
    strcpy(datoPartida->estado, "listo");
    datoPartida->jugadores=0;
    datoPartida->intentos=7;
    datoPartida->listos=5;
    datoPartida->rondas=0;
    datoPartida->letrasIngresadas[0]='\0';


    while (datoPartida->jugadores!=datoPartida->listos){
        usleep(200000);
    }


    sem_t *servidor = sem_open("servidor",O_CREAT,0600,0);
    sem_t *escritura = sem_open("escritura",O_CREAT,0600,0);
    cantidadJugadores = datoPartida->listos;

    inicializarSemaforos();

    for(int j = 0; j<cantLetrasPalabra; j++){
        strcpy(&datoPartida->palabraOculta[j],"-");
    }

    int pos;

    char *est = datoPartida->estado;
    string estado = est;
    bool flag = true;

    if(datoPartida->jugadores > 1){
        sem_post(servidor);
        while (estado == "ronda" && datoPartida->jugadores > 1){
            for (int i = 0; i<cantidadJugadores && flag; i++){
                sem_wait(servidor);
                sem_post(semaforos[i]);
                sem_wait(escritura);
                char a=*datoPartida->letraIngresada;
                if(esLetraPalabra(palabra, a)){
                    pos = palabra.find(a);
                    datoPartida->palabraOculta[pos]=a;
                    for (int w = pos+1; (w = palabra.find(a, w)) != std::string::npos; w++) {
                        datoPartida->palabraOculta[w]=a;
                    }
                    datoPartida->resultado=0;
                    jugadores[i]+=2;
                }
                else{
                    datoPartida->resultado=1;
                    datoPartida->intentos--;
                    jugadores[i]--;
                }
                datoPartida->puntos=jugadores[i];
                if(datoPartida->intentos == 0){
                    strcpy(datoPartida->estado,"finSG");
                    flag = false;
                }
                if(datoPartida->jugadores == 1){
                    flag=false;
                }
                if(!buscarGuion(datoPartida->palabraOculta)){
                    strcpy(datoPartida->estado,"finCG");
                    int ganador = buscarGanador(jugadores,cantidadJugadores);
                    datoPartida->ganador=ganador;
                    flag = false;
                }
                sem_post(semaforos[i]);
                est = datoPartida->estado;
                estado = est;
            }
        }
    }
    else{
        while (estado == "ronda"){
            sem_wait(servidor);
            char a=*datoPartida->letraIngresada;
            if(esLetraPalabra(palabra, a)){
                pos = palabra.find(a);
                datoPartida->palabraOculta[pos]=a;
                for (int w = pos+1; (w = palabra.find(a, w)) != std::string::npos; w++) {
                    datoPartida->palabraOculta[w]=a;
                }
                datoPartida->resultado=0;
                jugadores[0]+=2;
            }
            else{
                datoPartida->resultado=1;
                datoPartida->intentos--;
                jugadores[0]--;
            }
            datoPartida->puntos=jugadores[0];
            if(datoPartida->intentos == 0){
                strcpy(datoPartida->estado,"finSG");
            }
            if(!buscarGuion(datoPartida->palabraOculta)){
                strcpy(datoPartida->estado,"finCG");
                datoPartida->ganador=0;
            }
            sem_post(semaforos[0]);
            est = datoPartida->estado;
            estado = est;
        }
    }
    liberarSemaforos();
    usleep(1000000);
    cerrarEliminarSem();
    usleep(1000000);
    shmdt(datoPartida);
    shmctl(shmid,IPC_RMID,NULL);
    return 0;
}

void INThandler(int signum)
{
    strcpy(datoPartida->estado, "sinServer");
    liberarSemaforos();
    usleep(1000000);
    cerrarEliminarSem();
    shmdt(datoPartida);
    shmctl(shmid,IPC_RMID,NULL);
    exit(0);
}

void ctrlHandler(int signum){

}

string obtenerPalabraDeArchivo() {

    fstream palabrasFile;
    srand(time(nullptr));
    palabrasFile.open("./palabras.txt",ios::in);

    if(!palabrasFile) {
        cerr << "No se pudo abrir archivo" << endl;
        exit(1);
    }

    std::vector<string> palabras;
    string cadena = "";
    while(palabrasFile >> cadena) {
        palabras.push_back(cadena);
    }

    if(cadena == "")
        return nullptr;

    palabrasFile.close();

    return palabras[rand()%palabras.size()];
}


bool buscarGuion(char* palabra){
    char* copia = palabra;
    while(*copia != '\0'){
        if (*copia == '-'){
            return true;
        }
        copia++;
    }
    return false;
}

int buscarGanador(int * jugadores, int cant){
    int maximo, ganador;
    maximo=-20;
    for(int i = 0; i<cant; i++){
        if(jugadores[i]>maximo){
            maximo = jugadores[i];
            ganador=i;
        }
    }
    return ganador;
}


void cerrarEliminarSem(){
    for (int i = 0; i<cantidadJugadores; i++){
        char nombreSem[10] = "cliente";
        char nro[5];
        sprintf(nro,"%d",i);
        strcat(nombreSem, nro);
        sem_unlink("nombreSem");
    }
    sem_unlink("servidor");
    sem_unlink("escritura");
}

void liberarSemaforos(){
    for (int i = 0; i<cantidadJugadores; i++){
        sem_post(semaforos[i]);
    }
}

bool mostrarAyuda(const char *cad)
{
    if (!strcmp(cad, "-h") || !strcmp(cad, "--help") )
    {
        cout << "Este es el proceso servidor del juego ahorcado para 1-3 personas" << endl;
        cout << "    servidor - realiza logica del ahorcado" << endl;
        cout << "SYNOPSIS:" << endl;
        cout << "    servidor [--help -h]" << endl;
        cout << "DESCRIPTION:" << endl;
        cout << "    Encargado de ejecutar la logica en base a los clientes que se conecten" << endl;
        return true;
    }
    return false;
}

void inicializarSemaforos(){
    for (int i = 0; i<cantidadJugadores; i++){
        char nombreSem[10] = "cliente";
        char nro[5];
        sprintf(nro,"%d",i);
        strcat(nombreSem, nro);
        semaforos[i] = sem_open(nombreSem,O_CREAT,0600,0);
    }
}

bool esLetraPalabra(string palabra, char a){
    return palabra.find(a) != std::string::npos;
}
