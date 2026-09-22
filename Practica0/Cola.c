#include <EstructurasBasicas.h>
/*

            Cola

*/

struct Cola{
    struct Nodo *primero;
    struct Nodo *ultimo;
}cola;

/**
 * Función que inicializa la cola.
 */
void inicializarCola(void){
    cola.primero = NULL;
    cola.ultimo = NULL;
}

/**
 * Función que agrega un elemento a la cola.
 */
void enqueue(int elemento){
    struct Nodo *nuevo = crear_nodo(elemento);

    if (cola.primero == NULL){
        cola.primero = nuevo;
        cola.ultimo = nuevo;
    }else{
        cola.ultimo->siguiente = nuevo;
        cola.ultimo = nuevo;
    }
}

/**
 * Funcion que elimina y regresa al primer elemento de la cola.
 */
int dequeue(){
    if (cola.primero == NULL){
        return -1;
    }

    struct Nodo *primero = cola.primero;
    int elemento = primero->elemento;

    cola.primero = primero->siguiente;

    if (cola.primero == NULL){
        cola.ultimo = NULL;
    }

    free(primero);

    return elemento;
}

/**
 * Regresa 1 si la cola está vacía. 0 en otro caso.
 */
int emptyCola(){
    return (cola.primero == NULL)? 1: 0;
}

/**
 * Funcion que regresa el primer elemento de la cola.
 */
int first(void){
    return (emptyCola())? -1: cola.primero->elemento;
}

/**Función que imprime una representación de la cola. */
void printPile(void){
    struct Nodo *actual = cola.primero;

    while (actual != NULL) {

        if (actual == cola.primero) {
            printf(" FIRST-> %d\n", actual->elemento);
        } else {
            printf("         %d\n", actual->elemento);
        }
        actual = actual->siguiente;
    }
}