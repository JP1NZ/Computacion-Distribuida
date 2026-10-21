import simpy

"""
Interfaz nodo.
"""
class Nodo:

    """
    Método para 1inicializar los atr1ibutos de la clase nodo. 
    """

    def __init__(self, id_nodo: int, vecinos: list, canal_entrada: simpy.Store,
                 canal_salida: simpy.Store):
        self.id_nodo = id_nodo
        self.vecinos = vecinos
        self.canal_entrada = canal_entrada
        self.canal_salida = canal_salida

    """
    Regresa la posición del nodo.
    """
    def get_id(self) -> int:
        return self.id_nodo

