import time
import random
from mpi4py import MPI

comm = MPI.COMM_WORLD
rank = comm.Get_rank()
size = comm.Get_size()


# 1. Definición de Estados
SUSCEPTIBLE = "SUSCEPTIBLE"
INFECTED = "INFECTED"
REMOVED = "REMOVED"

# Tags para diferenciar los tipos de mensajes
TAG_PUSH = 1
TAG_REPLY = 2

# Variables iniciales (Estado base para todos)
state = SUSCEPTIBLE
value = None
k = 5          # Contador.
counter = 0
delta = 0.1    # Timeout (0.1 segundos) Tamano de la ronda.

# --- update(v) ---
# El nodo 0 será el origen del chisme
if rank == 0:
    value = "¡El rumor ha comenzado!"
    state = INFECTED
    counter = k

last_timeout = time.time()
start_simulation = time.time()

# BUCLE DE EVENTOS
# Como es una simulación distribuida, la dejamos correr por 3 segundos 
# para dar tiempo a que el rumor se propague a los 100+ nodos.
while time.time() - start_simulation < 3.0:

    # --- on timeout ---
    if state == INFECTED and (time.time() - last_timeout) >= delta:
        # q <- random(P) (que no sea yo mismo)
        q = random.choice([i for i in range(size) if i != rank])
        
        # send <PUSH, p, value> to q
        # Usamos comm.send para enviar un diccionario
        comm.send({'p': rank, 'v': value}, dest=q, tag=TAG_PUSH)
        
        # set timeout delta (reiniciar temporizador)
        last_timeout = time.time()

    # --- on receive <PUSH, q, v> ---
    # iprobe devuelve True si hay un mensaje de tipo PUSH esperando
    if comm.iprobe(source=MPI.ANY_SOURCE, tag=TAG_PUSH):
        msg = comm.recv(source=MPI.ANY_SOURCE, tag=TAG_PUSH)
        q_sender = msg['p']
        v_received = msg['v']
        
        # send <REPLY, state> to q (al que me mandó el push)
        comm.send({'s': state}, dest=q_sender, tag=TAG_REPLY)
        
        if state == SUSCEPTIBLE:
            value = v_received
            state = INFECTED
            counter = k
            last_timeout = time.time() # Inicia su propio timeout

    # --- on receive <REPLY, s> ---
    if comm.iprobe(source=MPI.ANY_SOURCE, tag=TAG_REPLY):
        msg = comm.recv(source=MPI.ANY_SOURCE, tag=TAG_REPLY)
        s_reply = msg['s']
        
        # Si me responden y yo sigo infectado...
        if s_reply != SUSCEPTIBLE and state == INFECTED:
            counter -= 1
            if counter == 0:
                state = REMOVED

# --- FIN DE LA SIMULACIÓN: RECOPILACIÓN DE DATOS ---
# Usamos gather para que el proceso 0 junte los estados finales y arme el reporte
estados_finales = comm.gather(state, root=0)

if rank == 0:
    total_susceptibles = estados_finales.count(SUSCEPTIBLE)
    total_infectados = estados_finales.count(INFECTED)
    total_removidos = estados_finales.count(REMOVED)
    
    print("=== REPORTE FINAL DE RUMOR MONGERING ===")
    print(f"Total de nodos (procesos) simulados: {size}")
    print(f"Nodos que nunca escucharon el rumor (SUSCEPTIBLES): {total_susceptibles}")
    print(f"Nodos que siguen esparciendo (INFECTADOS): {total_infectados}")
    print(f"Nodos que perdieron el interés (REMOVIDOS): {total_removidos}")
    print("========================================")
