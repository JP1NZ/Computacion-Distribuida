#include "busqueda.h"

#include "auxiliares.h"
#include "mensaje.h"

#include <stdlib.h>

typedef struct {
    bool coordinador; // Nos dice si el nodo es el nodo coordinador.
    bool iniciado; // Nos dice si la busqueda ya ha comenzado.
    bool finalizo; // Nos dice si la busqueda ya ha terminado.
    bool encontrado; // Nos dice si la busqueda logró encontrar al elemento.
    size_t esperados; //Cantidad de elementos esperados.
    size_t recibidos; // Cantidad de elementos recibidos después de realizar la busqueda.
    bool *respondio; // Almacena la información de si los nodo trabajadores responieron o no a la solicitud de busqueda.
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
    Funcion que realiza la busqueda de un elemento en el arreglo recibido por el nodo. Para la busqueda se 
    emplea busqueda binaria.
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

    (void)enviado; //Con esto no da avertencias el compilador :)
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

    // 1. Validar los argumentos. Simplemente para saber si continuar o no.
    if (nodo == NULL ||
        simulador == NULL ||
        nodo->estado == NULL ||
        (longitud > 0 && arreglo == NULL)) {
        return false;
    }

    // Creamo el estado de la busqueda. El nodo proporciona el estado inicialmente.
    EstadoBusqueda *estado = nodo->estado;

    // 2. De acuerdo con el estado proporcionado con el nodo, verificamos si el nodo es el nodo coordinador
    // Si no lo es no hacemos nada y regresamos true. 
    if (!estado->coordinador) {
        return true;
    }

    // Necesitamos al menos un trabajador. Si no hay trabajadores no se puede iniciar la distribución.
    if (nodo->cantidad_vecinos == 0) {
        return false;
    }

    // 3. Creamos la cuadricula par dividir el arreglo.
    Cuadricula cuadricula = {0};

    if (!cuadricula_crear(arreglo,
                          longitud,
                          nodo->cantidad_vecinos,
                          &cuadricula)) {
        return false;
    }

    // 4. Con la división el arreglo y las comprobaciones básicas hechas (es coordinador y tiene vecinos) 
    // podemos continuar actualizando el Estado de la busqueda.
    estado->encontrado = false;
    estado->recibidos = 0;
    estado->finalizo = false;

    // Inicialmente ningún nodo ha respondido a la solicitud e busqueda.
    for (size_t i = 0; i < estado->esperados; ++i) {
        estado->respondio[i] = false;
    }

    // 5. Enviar el mensaje con su segmento, a cada trabajador.
    for (size_t i = 0; i < nodo->cantidad_vecinos; ++i) {

        Mensaje mensaje = {0};

        // Crear el mensaje con el segmento correspondiente.
        if (!mensaje_crear(&mensaje,
                           MSG_BUSCAR_TRABAJO,
                           nodo->id,
                           nodo->id,
                           elemento,
                           cuadricula.segmentos[i], //Segmento
                           cuadricula.longitudes[i])) { // Longitu del segmento.
            
            //En caso de que el mensaje no se pueda construir.
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

        // Liberar el espacio del mensaje local.
        mensaje_destruir(&mensaje);
        

        //Si no se pudo enviar el mensaje, liberamos la memoria y regresamos false.
        if (!enviado) {
            cuadricula_destruir(&cuadricula);
            return false;
        }
    }

    // Ya se han enviado todos los mensajes. Liberamos memoria
    cuadricula_destruir(&cuadricula);

    //Actualizamos el estado de la busqueda a "iniciado".
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
