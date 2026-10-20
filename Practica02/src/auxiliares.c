#include "auxiliares.h"

#include <stdlib.h>
#include <string.h>

/*
    Recibe un apuntador a un arreglo de varios elementos (sobre el que haremos una busqueda u ordenaremos más tarde).
    longitud: del arreglo dado.
    cantidad_segmentos: corresponde a la cantidad de segmentos en los que se divide el arreglo, osea, el número de nodos trabajadores.
    Se recibe una cuadrícula que se modificará a lo largo de la ejecución de la función. 
*/
bool cuadricula_crear(const int *arreglo,
                      size_t longitud,
                      size_t cantidad_segmentos,
                      Cuadricula *salida) {

    //ERRORES
    if (salida == NULL || cantidad_segmentos == 0 ||
        (longitud > 0 && arreglo == NULL)) {
        return false;
    }

    //Reservamos memoria para los arreglos basándonos en la cantidad de segmentos.
    salida->segmentos = calloc(cantidad_segmentos, sizeof(*salida->segmentos));
    salida->longitudes = calloc(cantidad_segmentos, sizeof(*salida->longitudes));


    salida->cantidad = cantidad_segmentos;

    //ERRORES al reservar la memoria para los arreglos. 
    if (salida->segmentos == NULL || salida->longitudes == NULL) {
        cuadricula_destruir(salida);
        return false;
    }

    
    size_t base = longitud / cantidad_segmentos; //Tamano base (mínimo) de todos los segmentos.
    size_t residuo = longitud % cantidad_segmentos; //Tamano del residuo que debe distribuirse entre los segmentos.
    size_t inicio = 0; //Indice inicial del arreglo que vamos a partir.


    // Determinamos el tamano de los segmentos para cada nodo.
    // También construimos los propios segmentos con los elementos del arreglo original.
    for (size_t i = 0; i < cantidad_segmentos; ++i) {


        /*
            Todos los nodos tienen el tamano base, sin embargo existe un "residuo", un
            número de elementos que todavía no se han distribuido a ningún nodo . Sabemos que
            0 <= residuo < cantidad_segmentos por lo que podemos distribuir 1 por los elementos restantes, a los nodos.
        */
        size_t tamano = base + (i < residuo ? 1U : 0U); 

        // Guardamos en nuestro arreglo de longitudes la longitud del segmento que se asignará al nodo i.
        salida->longitudes[i] = tamano;

        //
        if (tamano > 0) {

            //Reservamos memoria para el segmento del nodo i que vamos a construir.
            salida->segmentos[i] = malloc(tamano * sizeof(*salida->segmentos[i]));

            //Error al reservar la memoria.
            if (salida->segmentos[i] == NULL) {
                cuadricula_destruir(salida);
                return false;
            }

            //Copiamos en el arreglo de segmentos, el segmento del arreglo original que le corresponde al nodo i.
            memcpy(salida->segmentos[i],
                   arreglo + inicio,
                   tamano * sizeof(*arreglo));
        }
        inicio += tamano;
    }
    return true;
}


/*
    Función que destruye los datos almacenados en una cuadrícula y los
    regresa a sus valores por defecto.

*/
void cuadricula_destruir(Cuadricula *cuadricula) {
    //No es una dirección válida. NO hacemos nada.
    if (cuadricula == NULL) {
        return;
    }

    // La cuadricula tiene un arreglo de segmentos. Se libera la memoria de cada posición del arreglo de segmentos.
    // Escencialmente se borran los segmentos de cada nodo.
    if (cuadricula->segmentos != NULL) {
        for (size_t i = 0; i < cuadricula->cantidad; ++i) {
            free(cuadricula->segmentos[i]);
        }
    }

    // El arreglo de segmentos existe con puros NULL. Se libera la memoria de este arreglo.
    free(cuadricula->segmentos);

    //Se libera la memoria del arreglo de longitudes (se elimina el arreglo de longitudes).
    free(cuadricula->longitudes);

    //Borramos las antiguas direcciones de memoria (ya no apuntan a nada útil).
    cuadricula->segmentos = NULL;
    cuadricula->longitudes = NULL;

    cuadricula->cantidad = 0;
}


/*
    cuadricula corresponde a una Cuadricula con los segmentos del arreglo ya distribuidos a su respectivos nodo (eso se espera).
    resultado es un arreglo que contiene los arreglos resultantes de las operaciones de cada nodo.
    longitud_resultado es la longitud de los arreglos resultantes de las operaciones de cada nodo.
*/
bool k_way_merge(const Cuadricula *cuadricula,
                 int **resultado,
                 size_t *longitud_resultado) {

    //Errores
    if (cuadricula == NULL || resultado == NULL || longitud_resultado == NULL) {
        return false;
    }

    // Se calcula el total de la suma de las longitudes de los arreglos que se van a combinar.
    size_t total = 0;
    for (size_t i = 0; i < cuadricula->cantidad; ++i) {
        total += cuadricula->longitudes[i];
    }

    //Tratamos de reservar memoria para el arreglo de salida.
    int *salida = total == 0 ? NULL : malloc(total * sizeof(*salida)); 

    /*  
        Intentamos reservar espacio para un arreglo que guarde las posiciones en que nos encontramos en cada segmento.
        Estas posiciones se inicializan como cero.
        Por ejemplo si posiciones[0] = 2 significa que en el primer nodo trabajador nos encontramos en la posición 2 de su segmento.
    */
    size_t *posiciones = calloc(cuadricula->cantidad, sizeof(*posiciones));

    //Errores al reservar memoria.
    if ((total > 0 && salida == NULL) ||
        (cuadricula->cantidad > 0 && posiciones == NULL)) {
        free(salida);
        free(posiciones);
        return false;
    }

    //Buscamos 
    for (size_t k = 0; k < total; ++k) {
        size_t elegido = cuadricula->cantidad; //Número de segmento elegido.
        
        /*
            Se comparan los elementos en las posiciones guardadas de todos los segmentos
            para determinar el segmento donde se encuentra el menor.
        */
        for (size_t i = 0; i < cuadricula->cantidad; ++i) {

            // Verificamos si la posicion en el segmento i todavía se encuentra dentro de los límites del segmento.
            if (posiciones[i] < cuadricula->longitudes[i] 
                
                /*
                    Además, se verifica si el segmento elegido sigue siendo el que se eligió al principio o bien
                    que el elemento en la posición guardada del segmento i sea menor que el elemento en la posición guardada 
                    del segmento elegido.
                */
                &&
                (elegido == cuadricula->cantidad ||
                 cuadricula->segmentos[i][posiciones[i]] <
                     cuadricula->segmentos[elegido][posiciones[elegido]])) {
                
                // De  esta  forma sabemos que en el segmento i la posición apunta a un elemento menor al que apunta la posicion de
                // elegido, asi que cambiamos el número de segmento elegido por i. 
                elegido = i;
            }
        }

        /*
            Terminadas todas las comparaciones, elegido es el segmento cuya posición guardada contiene al elemento más  de entre todas
            las demás posiciones guardadas de los demás segmentos.
            En el arreglo de salida guardamos en la posición k el valor del segmento. 
            Después, se aumenta la posición en 1, apuntando asi al siguiente elemento de ese segmento.
        */
        salida[k] = cuadricula->segmentos[elegido][posiciones[elegido]++];
        
    }

    free(posiciones);

    //El arreglo resultante es la salida.
    *resultado = salida;
    *longitud_resultado = total;

    return true; //Logramos completar el merge :)
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
    for (size_t i = 0; i < longitud; i++){
        if (elemento == arreglo[i]){
            return true;
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
