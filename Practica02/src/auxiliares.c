#include "auxiliares.h"

#include <stdlib.h>
#include <string.h>

/*
Recibe un arre
*/
bool cuadricula_crear(const int *arreglo,
                      size_t longitud,
                      size_t cantidad_segmentos,
                      Cuadricula *salida) {
    if (salida == NULL || cantidad_segmentos == 0 ||
        (longitud > 0 && arreglo == NULL)) {
        return false;
    }

    salida->segmentos = calloc(cantidad_segmentos, sizeof(*salida->segmentos));
    salida->longitudes = calloc(cantidad_segmentos, sizeof(*salida->longitudes));
    salida->cantidad = cantidad_segmentos;
    if (salida->segmentos == NULL || salida->longitudes == NULL) {
        cuadricula_destruir(salida);
        return false;
    }

    size_t base = longitud / cantidad_segmentos;
    size_t residuo = longitud % cantidad_segmentos;
    size_t inicio = 0;

    for (size_t i = 0; i < cantidad_segmentos; ++i) {
        size_t tamano = base + (i < residuo ? 1U : 0U);
        salida->longitudes[i] = tamano;
        if (tamano > 0) {
            salida->segmentos[i] = malloc(tamano * sizeof(*salida->segmentos[i]));
            if (salida->segmentos[i] == NULL) {
                cuadricula_destruir(salida);
                return false;
            }
            memcpy(salida->segmentos[i],
                   arreglo + inicio,
                   tamano * sizeof(*arreglo));
        }
        inicio += tamano;
    }
    return true;
}

void cuadricula_destruir(Cuadricula *cuadricula) {
    if (cuadricula == NULL) {
        return;
    }
    if (cuadricula->segmentos != NULL) {
        for (size_t i = 0; i < cuadricula->cantidad; ++i) {
            free(cuadricula->segmentos[i]);
        }
    }
    free(cuadricula->segmentos);
    free(cuadricula->longitudes);
    cuadricula->segmentos = NULL;
    cuadricula->longitudes = NULL;
    cuadricula->cantidad = 0;
}

bool k_way_merge(const Cuadricula *cuadricula,
                 int **resultado,
                 size_t *longitud_resultado) {
    if (cuadricula == NULL || resultado == NULL || longitud_resultado == NULL) {
        return false;
    }

    size_t total = 0;
    for (size_t i = 0; i < cuadricula->cantidad; ++i) {
        total += cuadricula->longitudes[i];
    }

    int *salida = total == 0 ? NULL : malloc(total * sizeof(*salida));
    size_t *posiciones = calloc(cuadricula->cantidad, sizeof(*posiciones));
    if ((total > 0 && salida == NULL) ||
        (cuadricula->cantidad > 0 && posiciones == NULL)) {
        free(salida);
        free(posiciones);
        return false;
    }

    for (size_t k = 0; k < total; ++k) {
        size_t elegido = cuadricula->cantidad;
        for (size_t i = 0; i < cuadricula->cantidad; ++i) {
            if (posiciones[i] < cuadricula->longitudes[i] &&
                (elegido == cuadricula->cantidad ||
                 cuadricula->segmentos[i][posiciones[i]] <
                     cuadricula->segmentos[elegido][posiciones[elegido]])) {
                elegido = i;
            }
        }
        salida[k] = cuadricula->segmentos[elegido][posiciones[elegido]++];
    }

    free(posiciones);
    *resultado = salida;
    *longitud_resultado = total;
    return true;
}


/*
Selection sort. 
Algoritmo sencillo de ordenamiento.
Recibe una puntero a un arreglo de enteros y la longitud de dicho arreglo.
*/
void ordenar_insercion(int *arreglo, size_t longitud) {
    //Ya esta ordenado trivialmente.
    if (longitud < 2) {
    return;
    }

    //Se ordena en otro caso. Hasta longitud - 1 porque para el último elemento del arreglo ya no es necesario intercambiar.
    for (size_t i = 0; i < longitud -1; i++ ){
        intercambiar(arreglo, i, busca_minimo(arreglo,i,longitud - 1));
    }
}


/*
Funcion auxiliar que busca al elemento mas pequeno de una determinada zona de un arreglo de enteros. 
a es el indice mas pequeno. Limite inferior.
b es el indice mas grande. Limite superior.
*/
size_t busca_minimo (int *arreglo, size_t a, size_t b){
    //Se verifica que a sea el limite inferior.
    if (a > b){
        exit(EXIT_FAILURE);
    }

    int minimo = arreglo[a];
    size_t posicion = a;
    for (size_t i = a + 1; i <= b ;i++){
        if (minimo > arreglo[i]){
            minimo = arreglo[i];
            posicion = i;
        }
    }
    return posicion;
}


/*
Función auxiliar que intercambia los elementos en las posiciones r y k de un arreglo de enteros.
*/
void intercambiar (int *arreglo, size_t r, size_t k){
    //No se verifica la validez de los índices pues se manda a llamar a esta función desde un for
    // que asegura su validez.


    //Copiamos el elemento en la posición r.
    int copia = arreglo[r];

    //Hacemos el intercambio.
    arreglo[r] = arreglo[k];
    arreglo[k] = copia;
}


/*
Algoritmo de busqueda lineal.
Regresa true si el elemento se encuentra en el arreglo dado, false en otro caso. 
*/
bool busqueda_lineal(const int *arreglo, size_t longitud, int elemento) {
    for (size_t i; i < longitud; i++){
        if (elemento == arreglo[i]){
            return true
        }
    }
    return false;

}


/*
Algoritmo de busqueda binaria.
Regresa true si el elemento se encuentra en el arreglo dado, false en otro caso. 
Esta versión de busqueda binaria es algo insulsa (de momento) puesto que tiene que hacer una copia
del arreglo recibido y ordenarlo. 
*/
bool busqueda_binaria(const int *arreglo, size_t longitud, int elemento) {
    //Longitud del arreglo es 0? JA TRIVIAL.
    if(longitud == 0){
       return false;
    }
    
    // Reservamos memoria para una copia del arreglo.
    int *copia = malloc(longitud * sizeof(int));

    if (copia == NULL) {
        return false; // No se pudo reservar memoria.
    }

    // Copiamos los elementos del arreglo original.
    memcpy(copia, arreglo, longitud * sizeof(int));
    
    // Ordenamos el arreglo copia;
    ordenar_insercion(copia, longitud);

    // Procedemos con el algoritmo de busqueda binaria en la copia.
    size_t max, mitad, min;
    min = 0;
    max = longitud -1;
    mitad = (max + min) /2;

    while (max - min > 1) {
        if (elemento < copia[mitad]){
            max = mitad;
            mitad = (max + min) /2;
        }else{
            min = mitad;
            mitad = (max + min) /2; 
        }
    }

    bool encontrado = copia[min] == elemento || copia[max] == elemento;
    free(copia);
    return encontrado;
}



int combinar_suma(int acumulado, int valor) {
    return acumulado + valor;
}
