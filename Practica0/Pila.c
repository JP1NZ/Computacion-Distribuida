#include <EstructurasBasicas.h>
/*

            PILA

*/

struct Pila{
    struct Nodo *tope;
} pila;


/**
 * Función que inicializa la pila.
 */
void inicializarPila(void){
    pila.tope = NULL;
}

/**
 * Función que agrega un elemento a la pila.
 */
void push (int elemento){
    struct Nodo *nuevo = crear_nodo(elemento);
        nuevo->siguiente = pila.tope;
        pila.tope = nuevo;
}

/**
 * Función que quita y regresa el elemento en el tope de la pila.
 */
int pop (){
    if(pila.tope == NULL){
        return -1;
    }else{
        struct Nodo *tope = pila.tope;
        pila.tope = pila.tope->siguiente;
        return tope->elemento;
    }

}

/**Función que nos dice si la pila está vacía. 
 * 1 si esta vacía, 0 en otro caso.
*/
int emptyPila(void){
    return (pila.tope == NULL)? 1 : 0; 
}

/**
 * Regresa el elemento en el tope de la pila.
 * Si la pila es vacía regresa NULL si no regresa el elemento.
 */
int top (void){
    return (emptyPila())? -1: pila.tope->elemento;
}

/**Función que imprime una representación de la pila. */
void printPile(void){
    struct Nodo *actual = pila.tope;

    while (actual != NULL) {

        if (actual == pila.tope) {
            printf("TOP -> %d\n", actual->elemento);
        } else {
            printf("       %d\n", actual->elemento);
        }
        actual = actual->siguiente;
    }
}