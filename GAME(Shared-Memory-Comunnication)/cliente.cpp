#include "./bibliotecas.hpp"

bool verificarEstado();

void INThandler(int);
void terminar();
int id;
char nombreSem[10]="cliente";
sem_t *escritura = sem_open("escritura",O_CREAT,0600,0);

partida *datoPartida;

bool validar(const char*);
bool esLetra(char);
bool esLetraIngresada(char*, char);
bool mostrarAyuda(const char*);

int main(int argc, char *argv[])
{
    signal(SIGINT, INThandler);
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

    // ftok to generate unique key
    key_t key = ftok("shmfile",65);
    // shmget returns an identifier in shmid
    int shmid = shmget(key,1024,0666|IPC_CREAT);
    // shmat to attach to shared memory
    datoPartida = (partida*) shmat(shmid,(void*)0,0);
    string letrasIngresadas;

    if(!verificarEstado()){
        terminar();
    }

    char nro[5];
    sprintf(nro,"%d",id);
    strcat(nombreSem, nro);

    sem_t *cliente = sem_open(nombreSem, O_CREAT, 0600, 0);
    sem_t *servidor = sem_open("servidor",O_CREAT,0600,0);


    char* est = datoPartida->estado;
    string estado = est;
    cout << "Bienvenidx a la partida. Una vez que todos los participantes esten listos comenzara" << endl << endl;

    if(datoPartida->jugadores > 1){
        while(estado == "ronda" && datoPartida->jugadores > 1){
            sem_wait(cliente);
            est = datoPartida->estado;
            estado = est;
            if(estado == "ronda"){
                cout << "Las letras ingresadas son: " << datoPartida->letrasIngresadas << endl;
                cout << "La palabra se ve como: " << datoPartida->palabraOculta << endl << endl;
                do{
                    cout << "por favor ingrese una letra" << endl;
                    cin >> datoPartida->letraIngresada;
                }while(!validar(datoPartida->letraIngresada) || esLetraIngresada(datoPartida->letrasIngresadas, *datoPartida->letraIngresada));
                datoPartida->letrasIngresadas[datoPartida->rondas]=datoPartida->letraIngresada[0];
                datoPartida->rondas++;
                datoPartida->letrasIngresadas[datoPartida->rondas]='\0';
                sem_post(servidor);
                sem_post(escritura);
                sem_wait(cliente);
                est = datoPartida->estado;
                estado = est;
                if(datoPartida->resultado == 0){
                    cout << "Bien! La letra ingresada esta en la palabra" << endl << endl;
                }
                else{
                    cout << "Mal! La letra ingresada no esta en la palabra" << endl << endl;
                }
                if(estado == "ronda"){
                    cout << "En este momento tenes: " << datoPartida->puntos << " puntos" << endl;
                    cout << "Restan: " << datoPartida->intentos << " intentos en total." << endl;
                }
            }
        }

        if(datoPartida->jugadores == 1){
            cout << "Se termino la partida. Ganaste por abandono" << endl;
        }
        if(estado == "sinServer"){
            cout << "Se termino la partida. El servidor fue cerrado" << endl;
        }
        if(estado == "finSG"){
            cout << "Se termino la partida. Todos han muerto ahorcados" << endl;
        }
        else if(estado == "finCG"){
            if(datoPartida->ganador == id){
                cout << "Se termino la partida. GANASTE!!!" << endl;
            }
            else{
                cout << "Se termino la partida. PERDISTE!!!" << endl;
            }
        }
    }
    else{
        while(estado == "ronda"){
            cout << "Las letras ingresadas son: " << datoPartida->letrasIngresadas << endl;
            cout << "La palabra se ve como: " << datoPartida->palabraOculta << endl << endl;
            do{
                cout << "por favor ingrese una letra" << endl;
                cin >> datoPartida->letraIngresada;
            }while(!validar(datoPartida->letraIngresada) || esLetraIngresada(datoPartida->letrasIngresadas, *datoPartida->letraIngresada));
            datoPartida->letrasIngresadas[datoPartida->rondas]=datoPartida->letraIngresada[0];
            datoPartida->rondas++;
            datoPartida->letrasIngresadas[datoPartida->rondas]='\0';
            sem_post(servidor);
            sem_wait(cliente);
            est = datoPartida->estado;
            estado = est;
            if(datoPartida->resultado == 0){
                cout << "Bien! La letra ingresada esta en la palabra" << endl << endl;
            }
            else{
                cout << "Mal! La letra ingresada no esta en la palabra" << endl << endl;
            }
            cout << "En este momento tenes: " << datoPartida->puntos << " puntos" << endl;
            cout << "Restan: " << datoPartida->intentos << " intentos en total." << endl;
        }
        if(datoPartida->intentos == 0){
            cout << "Te quedaste sin intentos, perdiste " << endl;
        }
        else{
            cout << "FELICITACIONES! Ganaste." << endl;
        }
    }

    sem_unlink("servidor");
    sem_unlink("escritura");
    sem_unlink(nombreSem);
    //detach from shared memory
    shmdt(datoPartida);
    return 0;
}

bool verificarEstado(){
    char* est = datoPartida->estado;
    string estado = est;
    if(estado != "listo"){
        if(estado == "lleno"){
            cout << "Servidor lleno, imposible conectarse" << endl;
            return false;
        }
        if(estado == "ronda"){
            cout << "Ya se comenzo una ronda. Espere para poder sumarse " << endl;
            return false;
        }
        else{
            cout << "El servidor no fue creado todavía" << endl;
            return false;
        }
    }
    else{
        id=datoPartida->jugadores;
        datoPartida->jugadores++;
        if(datoPartida->jugadores == 3){
            strcpy(datoPartida->estado, "lleno");
        }
    }
    string entrada;
    do{
        cout << "por favor escriba start para comenzar" << endl;
        cin >> entrada;
        if(entrada != "start"){
            cout << "ingreso desconocido" << endl;
            cout << endl;
        }
    }while(entrada!="start");
    strcpy(datoPartida->estado, "ronda");
    if(datoPartida->listos == 5){
        datoPartida->listos = 1;
    }
    else{
        datoPartida->listos++;
    }
    return true;
}

void INThandler(int signum)
{
    char *est = datoPartida->estado;
    string estado = est;
    datoPartida->jugadores--;
    if(datoPartida->jugadores == 1 && estado=="ronda"){
        strcpy(datoPartida->estado,"fin");
        sem_post(escritura);
    }
    if(estado == "lleno"){
        strcpy(datoPartida->estado,"listo");
    }
    sem_unlink("servidor");
    sem_unlink("escritura");
    sem_unlink(nombreSem);
    shmdt(datoPartida);
    exit(0);
}

void terminar(){
    shmdt(datoPartida);
    exit(0);
}


bool validar(const char* letra) {

    if(esLetra(*letra) && *(letra+1) == '\0')
        return true;

    cout << "Ingrese una sola letra y que sea valida!" << endl << endl;

    return false;
}

bool esLetra(char letra) {
    return (letra >= 65 && letra <= 90) || (letra >= 97 && letra <= 122);
}

bool esLetraIngresada(char* letras, char letra){
    for (int i = 0; *(letras + i) != '\0'; i++){
        if (*(letras + i) == letra){
            cout << "La letra ingresada ya está fue elegida. Elija otra" << endl << endl;
            return true;
        }
    }
    return false;
}

bool mostrarAyuda(const char *cad)
{
    if (!strcmp(cad, "-h") || !strcmp(cad, "--help") )
    {
        cout << "Este es el proceso cliente - ejecutable para iniciar ahorcado" << endl;
        cout << "SYNOPSIS:" << endl;
        cout << "    cliente [--help -h]" << endl;
        cout << "DESCRIPTION:" << endl;
        cout << "    Inicie el ejecutable para poder jugar al ahorcado" << endl;
        cout << "Una vez que un jugador decida escribir start ya no podran sumarse mas" << endl;
        return true;
    }
    return false;
}
