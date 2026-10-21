#ifndef AUXILIARES_H
#define AUXILIARES_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    int **segmentos; //Arreglo que almacena los segmentos del arreglo original, que recibe de cada nodo.
                        // Por ejemplo longitudes[1] = {1,2,4,1,6} nos dice que el nodo 1 recibe un segmento  {1,2,4,1,6} del arreglo original.

    size_t *longitudes; //Arreglo que almacena las longitudes de los segmentos que recibe de cada nodo.
                        // Por ejemplo longitudes[1] = 5 nos dice que el nodo 1 recibe un segmento de tamano 5 del arreglo original.

    size_t cantidad; //Cantidad de segmentos (numero de nodos).
} Cuadricula;

bool cuadricula_crear(const int *arreglo,
                      size_t longitud,
                      size_t cantidad_segmentos,
                      Cuadricula *salida);
void cuadricula_destruir(Cuadricula *cuadricula);

bool k_way_merge(const Cuadricula *cuadricula,
                 int **resultado,
                 size_t *longitud_resultado);

void ordenar_insercion(int *arreglo, size_t longitud);
bool busqueda_lineal(const int *arreglo, size_t longitud, int elemento);
size_t busca_minimo (int *arreglo, size_t a, size_t b);
void intercambiar (int *arreglo, size_t r, size_t k);
bool busqueda_binaria(const int *arreglo, size_t longitud, int elemento);
int combinar_suma(int acumulado, int valor);

#endif
