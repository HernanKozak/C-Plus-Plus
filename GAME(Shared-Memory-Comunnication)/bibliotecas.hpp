#ifndef ESCRITOR_HPP_INCLUDED
#define ESCRITOR_HPP_INCLUDED

#include <iostream>
#include <fstream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <cerrno>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>
#include <cstring>
#include <signal.h>
#include <vector>


typedef struct {
    int jugadores;
    int listos;
    int puntos;
    int intentos;
    int rondas;
    char palabraOculta[100];
    int resultado;
    int ganador;
    char letrasIngresadas[100];
    char letraIngresada[100];
    char estado[10];
}partida;

using namespace std;


#endif // ESCRITOR_HPP_INCLUDED
