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
        tiene_padre=False #Variable implementada para determinar si existe un padre

        if self.id_nodo == 0: #Pregunta por el nodo inicial
            msg = {"tipo": "GO", "d": -1, "de": self.id_nodo}# se crea el mensaje de -1 a si mismo
            yield self.canal_salida.envia(msg, [self.id_nodo])#se envia el mensaje a su propio buzon
        while True:
            msg = yield self.canal_entrada.get() #Pide la entrada del buzon 
            #print(env.now, self.id_nodo, msg) ambiente inicial

            if msg["tipo"] == "GO":#verifica si el mensaje es de seguimiento
                d = msg["d"]#estable a la variable d como la distancia que esta en el mensaje
                remitente = msg["de"]# guarda en remitente de quien fue el nodo que envio el mensaje

                if not tiene_padre or self.distancia > d + 1:   # (no tengo padre) o (la nueva distancia es el camino mas corto)
                    self.padre = remitente  # 1. referenciamos al padre
                    self.distancia = d + 1  # 2. referenciamos la distancia 
                    self.hijos = [] # 3. referenciamos los hijos
                    tiene_padre = True #tiene padre
                    #print(env.now, self.id_nodo, "padre:", self.padre, "dist:", self.distancia) "usado para imprimir los ambientes del bfs"

                    enviar_mensaje=[n for n in self.vecinos if n != remitente]    # 4. lista por comprension que incluye a los vecinos menos al remitiente
                    self.mensajes_esperados = len(enviar_mensaje)   # 5. Guarda los mensajes que faltan por llegar

                    if self.mensajes_esperados == 0:#Nadie espera el mensaje
                        respuesta = {"tipo": "BACK", "resp": True, "d": self.distancia, "de": self.id_nodo}#creamos mensaje hacia atras
                        yield self.canal_salida.envia(respuesta, [self.padre])#lo enviamos hacia el padre
                    else:
                        nuevo = {"tipo": "GO", "d": self.distancia, "de": self.id_nodo}#si si espera un vecino mensaje lo creamos
                        yield self.canal_salida.envia(nuevo, enviar_mensaje)# se lo evniamos al buzon de sus vecinos

                else:#El ndoo ya tienen padre y la distancia es mas larga que la guardada.
                    respuesta = {"tipo": "BACK", "resp": False, "d": d + 1, "de": self.id_nodo}#Se crea el mensaje
                    yield self.canal_salida.envia(respuesta, [remitente])#regresa uno hacia atras
            elif msg["tipo"]== "BACK":
                pass #Termina el proceso de llamada de mensajes si envia un BACK