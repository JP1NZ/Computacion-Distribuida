#include "broadcast.h"

#include "mensaje.h"

#include <stdlib.h>

typedef struct {
    int nodo_fuente;
    int mensaje;
    bool visto;
} EstadoBroadcast;

static void destruir_estado_broadcast(void *estado) {
    free(estado);
}

static void recibir_broadcast(Nodo *nodo,
                              const Mensaje *mensaje,
                              Simulador *simulador) {
    (void)nodo;
    (void)mensaje;
    (void)simulador;

    /*
     * TODO:
     * 1. Verificar que sea MSG_BROADCAST.
     * 2. Ignorarlo si el nodo ya vio el mensaje.
     * 3. Guardar el mensaje recibido.
     * 4. Marcarlo como visto.
     * 5. Reenviarlo a todos los vecinos.
     */
    
}

Nodo *nodo_broadcast_crear(int id,
                           const int *vecinos,
                           size_t cantidad_vecinos,
                           int nodo_fuente,
                           int mensaje_inicial) {
    EstadoBroadcast *estado = calloc(1, sizeof(*estado));
    if (estado == NULL) {
        return NULL;
    }
    estado->nodo_fuente = nodo_fuente;
    estado->mensaje = mensaje_inicial;
    estado->visto = false;

    Nodo *nodo = nodo_crear(id,
                            vecinos,
                            cantidad_vecinos,
                            recibir_broadcast,
                            estado,
                            destruir_estado_broadcast);
    if (nodo == NULL) {
        destruir_estado_broadcast(estado);
    }
    return nodo;
}

bool nodo_broadcast_iniciar(Nodo *nodo, Simulador *simulador) {
       (void)nodo;
    (void)simulador;

    /*
     * TODO:
     * 1. Comprobar si el nodo es la fuente.
     * 2. Marcar el mensaje como visto.
     * 3. Crear MSG_BROADCAST.
     * 4. Enviarlo a todos los vecinos.
     */

    return true;
}

bool nodo_broadcast_vio_mensaje(const Nodo *nodo) {
    if (nodo == NULL || nodo->estado == NULL) {
        return false;
    }
    const EstadoBroadcast *estado = nodo->estado;
    return estado->visto;
}

int nodo_broadcast_mensaje(const Nodo *nodo) {
    if (nodo == NULL || nodo->estado == NULL) {
        return 0;
    }
    const EstadoBroadcast *estado = nodo->estado;
    return estado->mensaje;
}
