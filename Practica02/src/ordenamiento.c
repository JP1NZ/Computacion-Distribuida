#include "ordenamiento.h"

#include "auxiliares.h"
#include "mensaje.h"

#include <stdlib.h>
#include <string.h>

typedef struct {
    bool coordinador;
    bool iniciado;
    bool finalizo;
    size_t esperados;
    size_t recibidos;
    bool *respondio;
    int **segmentos;
    size_t *longitudes;
    int *resultado;
    size_t longitud_resultado;
} EstadoOrdenamiento;

static void destruir_estado_ordenamiento(void *estado_generico) {
    EstadoOrdenamiento *estado = estado_generico;
    if (estado == NULL) {
        return;
    }
    if (estado->segmentos != NULL) {
        for (size_t i = 0; i < estado->esperados; ++i) {
            free(estado->segmentos[i]);
        }
    }
    free(estado->segmentos);
    free(estado->longitudes);
    free(estado->respondio);
    free(estado->resultado);
    free(estado);
}

static size_t indice_trabajador(const Nodo *nodo, int id) {
    for (size_t i = 0; i < nodo->cantidad_vecinos; ++i) {
        if (nodo->vecinos[i] == id) {
            return i;
        }
    }
    return nodo->cantidad_vecinos;
}

static void procesar_trabajo_ordenamiento(Nodo *nodo,
                                          const Mensaje *mensaje,
                                          Simulador *simulador) {
    int *segmento = NULL;
    if (mensaje->longitud > 0) {
        segmento = malloc(mensaje->longitud * sizeof(*segmento));
        if (segmento == NULL) {
            simulador->error_memoria = true;
            return;
        }
        memcpy(segmento,
               mensaje->datos,
               mensaje->longitud * sizeof(*segmento));
        ordenar_insercion(segmento, mensaje->longitud);
    }

    Mensaje respuesta;
    if (!mensaje_crear(&respuesta,
                       MSG_ORDENAR_RESULTADO,
                       nodo->id,
                       nodo->id,
                       0,
                       segmento,
                       mensaje->longitud)) {
        free(segmento);
        simulador->error_memoria = true;
        return;
    }
    free(segmento);

    if (!simulador_enviar(simulador,
                          mensaje->origen,
                          &respuesta,
                          TICK_ORDENAMIENTO)) {
        simulador->error_memoria = true;
    }
    mensaje_destruir(&respuesta);
}

static void procesar_resultado_ordenamiento(Nodo *nodo,
                                            const Mensaje *mensaje,
                                            Simulador *simulador) {
    EstadoOrdenamiento *estado = nodo->estado;
    size_t indice = indice_trabajador(nodo, mensaje->origen);
    if (indice == nodo->cantidad_vecinos || estado->respondio[indice]) {
        return;
    }

    if (mensaje->longitud > 0) {
        estado->segmentos[indice] = malloc(mensaje->longitud *
                                            sizeof(*estado->segmentos[indice]));
        if (estado->segmentos[indice] == NULL) {
            simulador->error_memoria = true;
            return;
        }
        memcpy(estado->segmentos[indice],
               mensaje->datos,
               mensaje->longitud * sizeof(*mensaje->datos));
    }
    estado->longitudes[indice] = mensaje->longitud;
    estado->respondio[indice] = true;
    ++estado->recibidos;

    if (estado->recibidos == estado->esperados) {
        Cuadricula vista = {
            .segmentos = estado->segmentos,
            .longitudes = estado->longitudes,
            .cantidad = estado->esperados
        };
        if (!k_way_merge(&vista,
                         &estado->resultado,
                         &estado->longitud_resultado)) {
            simulador->error_memoria = true;
            return;
        }
        estado->finalizo = true;
    }
}

static void recibir_ordenamiento(Nodo *nodo,
                                 const Mensaje *mensaje,
                                 Simulador *simulador) {
    EstadoOrdenamiento *estado = nodo->estado;
    if (!estado->coordinador && mensaje->tipo == MSG_ORDENAR_TRABAJO) {
        procesar_trabajo_ordenamiento(nodo, mensaje, simulador);
    } else if (estado->coordinador &&
               mensaje->tipo == MSG_ORDENAR_RESULTADO) {
        procesar_resultado_ordenamiento(nodo, mensaje, simulador);
    }
}

Nodo *nodo_ordenamiento_crear(int id,
                              const int *vecinos,
                              size_t cantidad_vecinos) {
    EstadoOrdenamiento *estado = calloc(1, sizeof(*estado));
    if (estado == NULL) {
        return NULL;
    }
    estado->coordinador = id == NODO_COORDINADOR;
    estado->esperados = estado->coordinador ? cantidad_vecinos : 0;

    if (estado->coordinador && cantidad_vecinos > 0) {
        estado->respondio = calloc(cantidad_vecinos, sizeof(*estado->respondio));
        estado->segmentos = calloc(cantidad_vecinos, sizeof(*estado->segmentos));
        estado->longitudes = calloc(cantidad_vecinos, sizeof(*estado->longitudes));
        if (estado->respondio == NULL || estado->segmentos == NULL ||
            estado->longitudes == NULL) {
            destruir_estado_ordenamiento(estado);
            return NULL;
        }
    }

    Nodo *nodo = nodo_crear(id,
                            vecinos,
                            cantidad_vecinos,
                            recibir_ordenamiento,
                            estado,
                            destruir_estado_ordenamiento);
    if (nodo == NULL) {
        destruir_estado_ordenamiento(estado);
    }
    return nodo;
}

bool nodo_ordenamiento_iniciar(Nodo *nodo,
                               Simulador *simulador,
                               const int *arreglo,
                               size_t longitud) {
    // 1. Validar los argumentos.
    if (nodo == NULL ||
        simulador == NULL ||
        nodo->estado == NULL ||
        (longitud > 0 && arreglo == NULL)) {
        return false;
    }

    EstadoOrdenamiento *estado = nodo->estado;

    // 2. Si es un trabajador, no tiene que iniciar nada.
    if (!estado->coordinador) {
        return true;
    }
    
    // Necesitamos al menos un trabajador.
    if (nodo->cantidad_vecinos == 0) {
        return false;
    }

    // 2. Dividir el arreglo en tantos segmentos como trabajadores.
    Cuadricula cuadricula = {0};

    if (!cuadricula_crear(arreglo,
                          longitud,
                          nodo->cantidad_vecinos,
                          &cuadricula)) {
        return false;
    }

    // La operación ya ha comenzado.
    estado->iniciado = true;

    // 3. Enviar un segmento a cada trabajador.
    for (size_t i = 0; i < cuadricula.cantidad; ++i) {

        Mensaje mensaje = {0};

        // Crear el mensaje con el segmento correspondiente.
        if (!mensaje_crear(&mensaje,
                           MSG_ORDENAR_TRABAJO,
                           nodo->id,
                           nodo->id,
                           0,
                           cuadricula.segmentos[i],
                           cuadricula.longitudes[i])) {

            cuadricula_destruir(&cuadricula);
            return false;
        }

        // Obtener el identificador del trabajador.
        int id_trabajador = nodo->vecinos[i];

        // Enviar el trabajo al trabajador correspondiente.
        bool enviado = simulador_enviar(simulador,
                                        id_trabajador,
                                        &mensaje,
                                        TICK_ORDENAMIENTO);

        // El simulador guarda su propia copia del mensaje.
        mensaje_destruir(&mensaje);

        if (!enviado) {
            cuadricula_destruir(&cuadricula);
            return false;
        }
    }

    // 4. Liberar la cuadrícula temporal.
    cuadricula_destruir(&cuadricula);

    return true;
}


bool nodo_ordenamiento_finalizo(const Nodo *nodo) {
    if (nodo == NULL || nodo->estado == NULL) {
        return false;
    }
    const EstadoOrdenamiento *estado = nodo->estado;
    return estado->finalizo;
}

const int *nodo_ordenamiento_resultado(const Nodo *nodo,
                                       size_t *longitud) {
    if (nodo == NULL || nodo->estado == NULL || longitud == NULL) {
        return NULL;
    }
    const EstadoOrdenamiento *estado = nodo->estado;
    *longitud = estado->longitud_resultado;
    return estado->resultado;
}
