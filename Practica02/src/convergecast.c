#include "convergecast.h"

#include "mensaje.h"

#include <stdlib.h>

typedef struct {
    int padre;
    int acumulado;
    size_t respuestas;
    bool *hijo_respondio;
    bool enviado;
    bool finalizo;
    FuncionCombinar combinar;
} EstadoConvergecast;

static void destruir_estado_convergecast(void *estado_generico) {
    EstadoConvergecast *estado = estado_generico;
    if (estado == NULL) {
        return;
    }
    free(estado->hijo_respondio);
    free(estado);
}

static bool enviar_acumulado(Nodo *nodo,
                             EstadoConvergecast *estado,
                             Simulador *simulador) {
    Mensaje mensaje;
    if (!mensaje_crear(&mensaje,
                       MSG_CONVERGECAST,
                       nodo->id,
                       nodo->id,
                       estado->acumulado,
                       NULL,
                       0)) {
        return false;
    }

    bool correcto = simulador_enviar(simulador,
                                      estado->padre,
                                      &mensaje,
                                      TICK_CONVERGECAST);
    mensaje_destruir(&mensaje);
    if (correcto) {
        estado->enviado = true;
    }
    return correcto;
}

static size_t indice_hijo(const Nodo *nodo, int id_hijo) {
    for (size_t i = 0; i < nodo->cantidad_vecinos; ++i) {
        if (nodo->vecinos[i] == id_hijo) {
            return i;
        }
    }
    return nodo->cantidad_vecinos;
}

static void recibir_convergecast(Nodo *nodo,
                                 const Mensaje *mensaje,
                                 Simulador *simulador) {
(void)nodo;
    (void)mensaje;
    (void)simulador;

    
    // 1. Validar los argumentos.
    if (nodo == NULL ||
        mensaje == NULL ||
        simulador == NULL ||
        nodo->estado == NULL) {
        return;
    }

    EstadoConvergecast *estado = nodo->estado;

    // 2. Comprobar el tipo de mensaje.
    if (mensaje->tipo != MSG_CONVERGECAST) {
        return;
    }

    // Un nodo finalizado no debe procesar más respuestas.
    if (estado->finalizo || estado->enviado) {
        return;
    }

    // 3. Identificar al hijo que envió la respuesta.
    size_t indice = indice_hijo(nodo, mensaje->origen);

    // Si el emisor no es uno de nuestros hijos, ignorar.
    if (indice == nodo->cantidad_vecinos) {
        return;
    }

    // 4. Evitar procesar dos veces la respuesta del mismo hijo.
    if (estado->hijo_respondio[indice]) {
        return;
    }

    estado->hijo_respondio[indice] = true;

    // 5. Combinar el valor recibido con el acumulado local.
    estado->acumulado = estado->combinar(
        estado->acumulado,
        mensaje->valor
    );

    // 6. Registrar que recibimos una respuesta.
    ++estado->respuestas;

    // Todavía faltan hijos por responder.
    if (estado->respuestas < nodo->cantidad_vecinos) {
        return;
    }

    // 7. Todos los hijos han respondido.
    bool es_raiz = estado->padre == nodo->id;

    if (es_raiz) {
        // La raíz ya tiene el resultado final.
        estado->finalizo = true;
    } else {
        // Enviar el acumulado al padre.
        if (!enviar_acumulado(nodo, estado, simulador)) {
            simulador->error_memoria = true;
            return;
        }

        estado->finalizo = true;
    }
}


Nodo *nodo_convergecast_crear(int id,
                              int padre,
                              const int *hijos,
                              size_t cantidad_hijos,
                              int valor_local,
                              FuncionCombinar combinar) {
    if (id < 0 || padre < 0 || combinar == NULL) {
        return NULL;
    }

    EstadoConvergecast *estado = calloc(1, sizeof(*estado));
    if (estado == NULL) {
        return NULL;
    }
    estado->padre = padre;
    estado->acumulado = valor_local;
    estado->combinar = combinar;

    if (cantidad_hijos > 0) {
        estado->hijo_respondio = calloc(cantidad_hijos,
                                        sizeof(*estado->hijo_respondio));
        if (estado->hijo_respondio == NULL) {
            destruir_estado_convergecast(estado);
            return NULL;
        }
    }

    Nodo *nodo = nodo_crear(id,
                            hijos,
                            cantidad_hijos,
                            recibir_convergecast,
                            estado,
                            destruir_estado_convergecast);
    if (nodo == NULL) {
        destruir_estado_convergecast(estado);
    }
    return nodo;
}

bool nodo_convergecast_iniciar(Nodo *nodo, Simulador *simulador) {
     (void)nodo;
    (void)simulador;

    // 1. Validar los argumentos.
    if (nodo == NULL ||
        simulador == NULL ||
        nodo->estado == NULL) {
        return false;
    }

    EstadoConvergecast *estado = nodo->estado;

    // Evitar iniciar dos veces el mismo nodo.
    if (estado->enviado || estado->finalizo) {
        return false;
    }

    // 2. Comprobar si el nodo es una hoja.
    bool es_hoja = nodo->cantidad_vecinos == 0;

    // 3. Comprobar si el nodo es la raíz.
    bool es_raiz = estado->padre == nodo->id;

    // 4. Si es raíz y hoja, el algoritmo termina.
    if (es_raiz && es_hoja) {
        estado->finalizo = true;
        return true;
    }

    // 5. Si es una hoja normal, enviar el acumulado al padre.
    if (es_hoja) {
        if (!enviar_acumulado(nodo, estado, simulador)) {
            return false;
        }

        estado->finalizo = true;
    }

    // 6. Si tiene hijos, esperará sus respuestas.
    return true;
}

bool nodo_convergecast_finalizo(const Nodo *nodo) {
    if (nodo == NULL || nodo->estado == NULL) {
        return false;
    }
    const EstadoConvergecast *estado = nodo->estado;
    return estado->finalizo;
}

int nodo_convergecast_resultado(const Nodo *nodo) {
    if (nodo == NULL || nodo->estado == NULL) {
        return 0;
    }
    const EstadoConvergecast *estado = nodo->estado;
    return estado->acumulado;
}
