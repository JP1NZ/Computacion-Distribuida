#include <EstructurasBasicas.h>
/*

            Cola

*/

struct Cola{
    int elemento;
    struct Nodo *primero;
}cola;

/**
 * Función que inicializa la cola.
 */
void inicializarCola(void){
    cola.primero = NULL;
}

/**
 * Función que agrega un elemento a la cola.
 */
void enqueue(int elemento){
    struct Nodo *nuevo = crear_nodo(elemento);
    if (cola.primero == NULL){
        cola.primero = nuevo;
    }else{
        cola.primero->siguiente =nuevo;
    }
   
}

/**
 * Funcion que elimina y regresa al primer elemento de la cola.
 */
int dequeue(){
    if (cola.primero == NULL){
        return NULL;
    }else{
        struct Nodo *primero = cola.primero;
        cola.primero = cola.primero->siguiente;
        return primero->elemento;
    }
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