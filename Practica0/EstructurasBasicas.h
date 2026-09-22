#include <stdio.h>
#include <stdlib.h>
#ifndef NODO_H
#define NODO_H

/* Estrcutura que define un nodo.*/
struct Nodo{
    int elemento;
    struct Nodo *siguiente;   // Puntero que apunta al nodo siguiente.
};

struct Nodo* crear_nodo(int elemento){
    // Le decimos al compilador de C que reserve en el HEAP una dirección para nuevo. 
    // De no hacer esto, la dirección de nuevo se borraría al terminar de ejecutar la funcion (por ser variable local).
    // Asi conservamos esta dirección.
    struct Nodo *nuevo = malloc(sizeof(struct Nodo)); 
    nuevo->elemento = elemento;
    nuevo->siguiente = NULL;
    return nuevo; //Regresamos el puntero al nuevo nodo.
}
#endif
