#include "busqueda.h"

#include "auxiliares.h"
#include "mensaje.h"

#include <stdlib.h>

typedef struct {
    bool coordinador;
    bool iniciado;
    bool finalizo;
    bool encontrado;
    size_t esperados;
    size_t recibidos;
    bool *respondio;
} EstadoBusqueda;

static void destruir_estado_busqueda(void *estado_generico) {
    EstadoBusqueda *estado = estado_generico;
    if (estado == NULL) {
        return;
    }
    free(estado->respondio);
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

/**
    Funcion que realiza la busqueda de un elemento en el arreglo recibido por el nodo.
    nodo : que realiza la bsuqueda.
    mensaje : recibido que contiene el elemento a buscar asi como el arreglo en donde buscar.
    simulador : que utilizameros para enviar la respuesta.
*/
static void procesar_trabajo_busqueda(
    Nodo *nodo,
    const Mensaje *mensaje,
    Simulador *simulador) {

    // 1. Buscar el elemento en el segmento recibido.
    bool encontrado = busqueda_binaria(mensaje->datos,mensaje-> longitud, mensaje-> valor);

    // 2. Crear el mensaje de respuesta.
    Mensaje respuesta = {0};

    if (!mensaje_crear(&respuesta,
                       MSG_BUSCAR_RESULTADO,
                       nodo->id,
                       mensaje->origen,
                       encontrado ? 1 : 0, //Valor
                       NULL,
                       0)) {
        return;
    }

    // 3. Enviar la respuesta al coordinador.
    bool enviado = simulador_enviar(simulador,
                                    respuesta.referente,
                                    &respuesta,
                                    1);

    // 4. Liberar la memoria del mensaje local.
    mensaje_destruir(&respuesta);

    (void)enviado;
}


static void procesar_resultado_busqueda(Nodo *nodo,
                                        const Mensaje *mensaje) {
    EstadoBusqueda *estado = nodo->estado;
    size_t indice = indice_trabajador(nodo, mensaje->origen);
    if (indice == nodo->cantidad_vecinos || estado->respondio[indice]) {
        return;
    }

    estado->respondio[indice] = true;
    ++estado->recibidos;
    estado->encontrado = estado->encontrado || mensaje->valor != 0;
    estado->finalizo = estado->recibidos == estado->esperados;
}

static void recibir_busqueda(Nodo *nodo,
                             const Mensaje *mensaje,
                             Simulador *simulador) {
    EstadoBusqueda *estado = nodo->estado;
    if (!estado->coordinador && mensaje->tipo == MSG_BUSCAR_TRABAJO) {
        procesar_trabajo_busqueda(nodo, mensaje, simulador);
    } else if (estado->coordinador &&
               mensaje->tipo == MSG_BUSCAR_RESULTADO) {
        procesar_resultado_busqueda(nodo, mensaje);
    }
}

Nodo *nodo_busqueda_crear(int id,
                          const int *vecinos,
                          size_t cantidad_vecinos) {
    EstadoBusqueda *estado = calloc(1, sizeof(*estado));
    if (estado == NULL) {
        return NULL;
    }
    estado->coordinador = id == 0;
    estado->esperados = estado->coordinador ? cantidad_vecinos : 0;

    if (estado->coordinador && cantidad_vecinos > 0) {
        estado->respondio = calloc(cantidad_vecinos, sizeof(*estado->respondio));
        if (estado->respondio == NULL) {
            destruir_estado_busqueda(estado);
            return NULL;
        }
    }

    Nodo *nodo = nodo_crear(id,
                            vecinos,
                            cantidad_vecinos,
                            recibir_busqueda,
                            estado,
                            destruir_estado_busqueda);
    if (nodo == NULL) {
        destruir_estado_busqueda(estado);
    }
    return nodo;
}


/*
    Inicia la distribución de la busqueda desde un nodo.
    nodo: es el nodo coordinador.
    Se nos proporciona un simulador.
    arreglo: es el arreglo en el que se quiere buscar.
    longitud: es la longitud del arreglo en el que se quiere buscar.
    elmento: a buscar en el arreglo
    Regresa true si se pudó iniciar la distribución.
*/
bool nodo_busqueda_iniciar(Nodo *nodo,
                           Simulador *simulador,
                           const int *arreglo,
                           size_t longitud,
                           int elemento) {

       // 1. Validar los argumentos.
    if (nodo == NULL ||
        simulador == NULL ||
        nodo->estado == NULL ||
        (longitud > 0 && arreglo == NULL)) {
        return false;
    }

    EstadoBusqueda *estado = nodo->estado;

    // 2. Si es un trabajador, no tiene que iniciar nada.
    if (!estado->coordinador) {
        return true;
    }

    // Necesitamos al menos un trabajador.
    if (nodo->cantidad_vecinos == 0) {
        return false;
    }

    // 2. Creamos la cuadricula.
    Cuadricula cuadricula = {0};

    if (!cuadricula_crear(arreglo,
                          longitud,
                          nodo->cantidad_vecinos,
                          &cuadricula)) {
        return false;
    }

    // 3. Inicializar el estado de la búsqueda.
    estado->encontrado = false;
    estado->recibidos = 0;
    estado->finalizo = false;

    for (size_t i = 0; i < estado->esperados; ++i) {
        estado->respondio[i] = false;
    }

    // 4. Enviar un segmento a cada trabajador.
    for (size_t i = 0; i < nodo->cantidad_vecinos; ++i) {

        Mensaje mensaje = {0};

        // Crear el mensaje con el segmento correspondiente.
        if (!mensaje_crear(&mensaje,
                           MSG_BUSCAR_TRABAJO,
                           nodo->id,
                           nodo->id,
                           elemento,
                           cuadricula.segmentos[i],
                           cuadricula.longitudes[i])) {

            cuadricula_destruir(&cuadricula);
            return false;
        }

        // Obtener el identificador del trabajador.
        int id_trabajador = nodo->vecinos[i];

        // Enviar el mensaje.
        bool enviado = simulador_enviar(simulador,
                                        id_trabajador,
                                        &mensaje,
                                        1);

        // Liberar el mensaje local.
        mensaje_destruir(&mensaje);

        if (!enviado) {
            cuadricula_destruir(&cuadricula);
            return false;
        }
    }

    // 5. Liberar la cuadrícula.
    cuadricula_destruir(&cuadricula);

    estado->iniciado = true;

    return true;
}
    


bool nodo_busqueda_finalizo(const Nodo *nodo) {
    if (nodo == NULL || nodo->estado == NULL) {
        return false;
    }
    const EstadoBusqueda *estado = nodo->estado;
    return estado->finalizo;
}

bool nodo_busqueda_resultado(const Nodo *nodo) {
    if (nodo == NULL || nodo->estado == NULL) {
        return false;
    }
    const EstadoBusqueda *estado = nodo->estado;
    return estado->encontrado;
}
