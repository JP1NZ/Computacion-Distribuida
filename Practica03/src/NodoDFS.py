import simpy
from Nodo import Nodo

TICK = 1

class NodoDFS(Nodo):

    def __init__(self, id_nodo, vecinos, canal_entrada, canal_salida):
        super().__init__(id_nodo, vecinos, canal_entrada, canal_salida)
        self.padre = self.id_nodo
        self.hijos = []
        self.visitado = False
        self.vecinos_pendientes = sorted(list(self.vecinos))
        self.terminado = False

    def dfs(self, env):
        ''' Algoritmo DFS distribuido determinista '''

        # 1. El nodo 0 arranca la exploración
        if self.id_nodo == 0:
            self.visitado = True
            if self.vecinos_pendientes:
                siguiente = self.vecinos_pendientes.pop(0)
                yield env.timeout(TICK)
                yield self.canal_salida.envia(("GO", self.id_nodo), [siguiente])
            else:
                self.terminado = True

        # 2. Bucle principal (Sin 'break')
        while True:
            msg = yield self.canal_entrada.get()
            tipo_msg = msg[0]
            id_emisor = msg[1]

            # --- MENSAJE "GO" ---
            if tipo_msg == "GO":
                if not self.visitado:
                    # Es la primera vez que visitan este nodo
                    self.visitado = True
                    self.padre = id_emisor
                    
                    # Quitamos a nuestro padre de los vecinos a explorar
                    if self.padre in self.vecinos_pendientes:
                        self.vecinos_pendientes.remove(self.padre)
                    
                    # Exploramos al siguiente vecino
                    if self.vecinos_pendientes:
                        siguiente = self.vecinos_pendientes.pop(0)
                        yield env.timeout(TICK)
                        yield self.canal_salida.envia(("GO", self.id_nodo), [siguiente])
                    else:
                        # Si no hay vecinos, somos nodo hoja
                        self.terminado = True
                        yield env.timeout(TICK)
                        yield self.canal_salida.envia(("BACK", self.id_nodo), [self.padre])
                else:
                    # ¡El nodo ya fue visitado! Enviamos REJECT
                    yield env.timeout(TICK)
                    yield self.canal_salida.envia(("REJECT", self.id_nodo), [id_emisor])

            # --- MENSAJE "BACK" ---
            elif tipo_msg == "BACK":
                # Un hijo completó su subárbol exitosamente
                self.hijos.append(id_emisor)
                
                # Seguimos explorando si nos quedan vecinos
                if self.vecinos_pendientes:
                    siguiente = self.vecinos_pendientes.pop(0)
                    yield env.timeout(TICK)
                    yield self.canal_salida.envia(("GO", self.id_nodo), [siguiente])
                else:
                    if not self.terminado:
                        self.terminado = True
                        if self.id_nodo != 0:
                            yield env.timeout(TICK)
                            yield self.canal_salida.envia(("BACK", self.id_nodo), [self.padre])

            # --- MENSAJE "REJECT" ---
            elif tipo_msg == "REJECT":
                # El vecino ya estaba en el árbol, pasamos al siguiente
                if self.vecinos_pendientes:
                    siguiente = self.vecinos_pendientes.pop(0)
                    yield env.timeout(TICK)
                    yield self.canal_salida.envia(("GO", self.id_nodo), [siguiente])
                else:
                    if not self.terminado:
                        self.terminado = True
                        if self.id_nodo != 0:
                            yield env.timeout(TICK)
                            yield self.canal_salida.envia(("BACK", self.id_nodo), [self.padre])