import simpy
from Nodo import *
from Canales.CanalRecorridos import *

# La unidad de tiempo
TICK = 1


class NodoBFS(Nodo):
    ''' Implementa la interfaz de Nodo para el algoritmo de Broadcast.'''

    def __init__(self, id_nodo, vecinos, canal_entrada, canal_salida):
        ''' Constructor de nodo que implemente el algoritmo BFS. '''
        super().__init__(id_nodo, vecinos, canal_entrada, canal_salida)
        self.padre = id_nodo
        self.hijos = []
        self.distancia = 0
        self.mensajes_esperados = 0

    def bfs(self, env):
        ''' Algoritmo BFS. '''
        #implementacion  
        tiene_padre=False
        if self.id_nodo == 0:
            msg = {"tipo": "GO", "d": -1, "de": self.id_nodo}
            yield self.canal_salida.envia(msg, [self.id_nodo])

        while True:
            msg = yield self.canal_entrada.get()
            print(env.now, self.id_nodo, msg)

            if msg["tipo"] == "GO":
                d = msg["d"]
                remitente = msg["de"]

                if not tiene_padre or self.distancia > d + 1:   # <-- REEMPLAZA por: (no tengo padre) or (camino más corto)
                    pass    # <-- borra este pass cuando rellenes el bloque
                    self.padre = remitente  # 1. self.padre = ...
                    self.distancia = d + 1  # 2. self.distancia = ...   
                    self.hijos = [] # 3. self.hijos = []
                    tiene_padre = True
                    print(env.now, self.id_nodo, "padre:", self.padre, "dist:", self.distancia)

                    enviar_mensaje=[n for n in self.vecinos if n != remitente]    # 4. vecinos a los que preguntar (menos el remitente)
                    self.mensajes_esperados = len(enviar_mensaje)   # 5. self.mensajes_esperados = cuántos son

                    if self.mensajes_esperados == 0:
                        respuesta = {"tipo": "BACK", "resp": True, "d": self.distancia, "de": self.id_nodo}
                        yield self.canal_salida.envia(respuesta, [self.padre])
                    else:
                        # GO a la lista enviar_mensaje con tu nivel
                        nuevo = {"tipo": "GO", "d": self.distancia, "de": self.id_nodo}
                        yield self.canal_salida.envia(nuevo, enviar_mensaje)
                else:
                    respuesta = {"tipo": "BACK", "resp": False, "d": d + 1, "de": self.id_nodo}
                    yield self.canal_salida.envia(respuesta, [remitente])
            elif msg["tipo"]== "BACK":
                pass #paso 3