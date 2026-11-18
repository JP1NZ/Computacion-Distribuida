from mpi4py import MPI
import time
import random

# ==========================================
# 1. FUNCIONES PARA DIFUSIÓN DE VALOR (ANTI-ENTROPÍA)
# ==========================================
# El estado guarda el valor y su timestamp: {'value': string, 'timestamp': float}

def selectNeighbor(my_rank, total_nodes):
    # q <- selectNeighbor()
    target = my_rank
    while target == my_rank:
        target = random.randint(0, total_nodes - 1)
    return target

def prepareRequest(state, q):
    # msg <- prepareRequest(state, q)
    # Solicitamos la sincronización enviando nuestro estado actual
    return state.copy()

def prepareReply(state, req, q):
    # rep <- prepareReply(state, req, q)
    # Respondemos al mensaje 'req' de 'q' enviando nuestro estado actual
    return state.copy()

def mergeRequest(state, req, q):
    # state <- mergeRequest(state, req, q)
    # Procesamos el request de 'q': si su valor es más nuevo, nos actualizamos
    if req['timestamp'] > state['timestamp']:
        return req.copy()
    return state.copy()

def mergeReply(state, rep, q):
    # state <- mergeReply(state, rep, q)
    # Procesamos la respuesta de 'q': si su valor es más nuevo, nos actualizamos
    if rep['timestamp'] > state['timestamp']:
        return rep.copy()
    return state.copy()


# ==========================================
# 2. ESQUEMA GENERAL (ALGORITMO 3)
# ==========================================
comm = MPI.COMM_WORLD
rank = comm.Get_rank()
size = comm.Get_size()

TAG_REQUEST = 1
TAG_REPLY = 2
delta = 0.1
SIMULATION_TIME = 2.0

# -- on initialization --
# state is initialized
if rank == 0:
    state = {'value': "¡Valor Actualizado v2.0!" , 'timestamp': time.time()}
else:
    state = {'value': "valor_viejo", 'timestamp': 0.0}

start_simulation = time.time()
# set timeout \Delta
last_timeout = start_simulation - delta 

# Ciclo asíncrono
while time.time() - start_simulation < SIMULATION_TIME:
    
    # -- on timeout --
    if (time.time() - last_timeout) >= delta:
        q = selectNeighbor(rank, size)
        msg = prepareRequest(state, q)
        
        # send <REQUEST, msg, p> to q
        comm.send(msg, dest=q, tag=TAG_REQUEST)
        # set timeout \Delta
        last_timeout = time.time()

    # -- Recepción de mensajes --
    status = MPI.Status()
    if comm.iprobe(source=MPI.ANY_SOURCE, tag=MPI.ANY_TAG, status=status):
        # 'sender' representa a la variable 'q' (el nodo vecino) en el pseudocódigo
        sender = status.Get_source()
        tag = status.Get_tag()
        
        if tag == TAG_REQUEST:
            # -- on receive <REQUEST, req, q> --
            req = comm.recv(source=sender, tag=TAG_REQUEST)
            rep = prepareReply(state, req, sender)
            
            # send <REPLY, rep, p> to q
            comm.send(rep, dest=sender, tag=TAG_REPLY)
            
            state = mergeRequest(state, req, sender)
            
        elif tag == TAG_REPLY:
            # -- on receive <REPLY, rep, q> --
            rep = comm.recv(source=sender, tag=TAG_REPLY)
            state = mergeReply(state, rep, sender)

# ==========================================
# 3. RECOLECCIÓN DE RESULTADOS
# ==========================================
final_states = comm.gather(state, root=0)

if rank == 0:
    print("\n--- RESULTADOS DE DIFUSIÓN DE VALOR ---")
    for i, final_state in enumerate(final_states):
        print(f"Nodo {i} tiene el valor: '{final_state['value']}'")
