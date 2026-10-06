#include "vecinos.h"

#include "mensaje.h"

#include <stdlib.h>

typedef struct {
    ConjuntoInt identificadores;
} EstadoVecinos;

static void destruir_estado_vecinos(void *estado_generico) {
    EstadoVecinos *estado = estado_generico;
    if (estado == NULL) {
        return;
    }
    conjunto_destruir(&estado->identificadores);
    free(estado);
}

//Implementar
static void recibir_myname(Nodo *nodo,
                           const Mensaje *mensaje,
                           Simulador *simulador) {
   (void)nodo;
    (void)mensaje;
    (void)simulador;
    /*
     * TODO:
     * 1. Verificar que el mensaje sea MSG_MYNAME.
     * 2. Obtener EstadoVecinos.
     * 3. Agregar al conjunto cada identificador recibido.
     */

}

Nodo *nodo_vecinos_crear(int id,
                         const int *vecinos,
                         size_t cantidad_vecinos) {
    EstadoVecinos *estado = malloc(sizeof(*estado));
    if (estado == NULL) {
        return NULL;
    }
    conjunto_inicializar(&estado->identificadores);

    Nodo *nodo = nodo_crear(id,
                            vecinos,
                            cantidad_vecinos,
                            recibir_myname,
                            estado,
                            destruir_estado_vecinos);
    if (nodo == NULL) {
        destruir_estado_vecinos(estado);
    }
    return nodo;
}

bool nodo_vecinos_iniciar(Nodo *nodo, Simulador *simulador) {
        (void)nodo;
    (void)simulador;

    /*
     * TODO:
     * 1. Crear un mensaje MSG_MYNAME.
     * 2. Colocar como payload la lista de vecinos del nodo.
     * 3. Enviar el mensaje a todos sus vecinos.
     * 4. Liberar el mensaje temporal.
     */

    return true;

}

const ConjuntoInt *nodo_vecinos_identificadores(const Nodo *nodo) {
    if (nodo == NULL || nodo->estado == NULL) {
        return NULL;
    }
    const EstadoVecinos *estado = nodo->estado;
    return &estado->identificadores;
}
