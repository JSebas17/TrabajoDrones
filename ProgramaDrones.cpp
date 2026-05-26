#include <iostream>
#include <string>
#include <sstream> // Necesario para to_string alternativo en C++ antiguo

using namespace std;

// Nodo para el Gestor de Procesos y Planificador de CPU (Lista Enlazada y Cola de Prioridad)
struct NodoProceso {
    int id;
    string nombre;
    int priority; // Cambiado un poco el nombre para evitar conflictos: 3: Alta (Emergencia), 2: Media (Operación), 1: Baja (Rutina)
    int memoria;   // Memoria requerida en MB
    NodoProceso* siguiente;
};

// Nodo para el Gestor de Memoria (Pila Dinámica - Stack)
struct NodoBloqueMemoria {
    int idProcesoAsignado;
    int cantidadMemoria;
    NodoBloqueMemoria* anterior; // Enlace inverso clásico de pilas
};

// ============================================================================
// VARIABLES GLOBALES (PUNTEROS DE CONTROL USANDO NULL)
// ============================================================================
NodoProceso* listaPrincipal = NULL; // Cabeza del Historial Global
NodoProceso* colaCPU = NULL;       // Frente de la Cola de Prioridad
NodoBloqueMemoria* topeMemoria = NULL; // Tope de la Pila de RAM
int ramTotalConsumida = 0;             // Contador auxiliar de uso de RAM

// Función auxiliar para convertir int a string compatible con C++ antiguo sin usar std::to_string
string convertirEnteroAString(int numero) {
    stringstream ss;
    ss << numero;
    return ss.str();
}

// ============================================================================
// COMPONENTE 3: GESTOR DE MEMORIA (PILA DINÁMICA - COMPLEJIDAD O(1))
// ============================================================================

void pushMemoria(int idProceso, int cantidad) {
    NodoBloqueMemoria* nuevoBloque = new NodoBloqueMemoria();
    nuevoBloque->idProcesoAsignado = idProceso;
    nuevoBloque->cantidadMemoria = cantidad;
    
    nuevoBloque->anterior = topeMemoria;
    topeMemoria = nuevoBloque;
    
    ramTotalConsumida += cantidad;
    cout << "[RAM] -> Bloque de " << cantidad << " MB asignado al Proceso ID " << idProceso << " (O(1))." << endl;
}

void popMemoria() {
    if (topeMemoria == NULL) {
        cout << "[RAM] Alerta: No hay bloques de memoria asignados que liberar." << endl;
        return;
    }
    
    NodoBloqueMemoria* bloqueAEliminar = topeMemoria;
    ramTotalConsumida -= bloqueAEliminar->cantidadMemoria;
    
    cout << "[RAM] <- Liberados " << bloqueAEliminar->cantidadMemoria 
         << " MB del Proceso ID " << bloqueAEliminar->idProcesoAsignado << " (O(1))." << endl;
         
    topeMemoria = topeMemoria->anterior; 
    delete bloqueAEliminar;              
}

void verificarEstadoMemoria() {
    cout << "\n=============================================" << endl;
    cout << "      ESTADO DE LA MEMORIA RAM DEL DRON      " << endl;
    cout << "=============================================" << endl;
    cout << "Memoria RAM Total en Uso: " << ramTotalConsumida << " MB" << endl;
    
    if (topeMemoria == NULL) {
        cout << "-> RAM limpia. Sin bloques activos." << endl;
        return;
    }
    
    NodoBloqueMemoria* temp = topeMemoria;
    cout << "Mapa de la Pila (Desde el Tope [LIFO]):" << endl;
    while (temp != NULL) {
        cout << "   [ Proceso ID: " << temp->idProcesoAsignado 
             << " | RAM: " << temp->cantidadMemoria << " MB ]" << endl;
        temp = temp->anterior;
    }
    cout << "=============================================" << endl;
}

// ============================================================================
// COMPONENTE 2: PLANIFICADOR DE CPU (COLA DE PRIORIDAD DINÁMICA)
// ============================================================================

void encolarPorPrioridad(int id, string nombre, int prioridad, int memoria) {
    NodoProceso* nuevoNodo = new NodoProceso();
    nuevoNodo->id = id;
    nuevoNodo->nombre = nombre;
    nuevoNodo->priority = prioridad;
    nuevoNodo->memoria = memoria;
    nuevoNodo->siguiente = NULL;
    
    if (colaCPU == NULL || prioridad > colaCPU->priority) {
        nuevoNodo->siguiente = colaCPU;
        colaCPU = nuevoNodo;
    } 
    else {
        NodoProceso* temp = colaCPU;
        while (temp->siguiente != NULL && temp->siguiente->priority >= prioridad) {
            temp = temp->siguiente;
        }
        nuevoNodo->siguiente = temp->siguiente;
        temp->siguiente = nuevoNodo;
    }
}

void desencolarCPU() {
    if (colaCPU == NULL) {
        cout << "[CPU] El planificador no registra tareas pendientes. Motores estables." << endl;
        return;
    }
    
    NodoProceso* procesoDespachado = colaCPU;
    colaCPU = colaCPU->siguiente;
    
    cout << "\n>>> [EJECUCIÓN] El procesador está ejecutando la tarea: " << procesoDespachado->nombre << endl;
    cout << "    ID: " << procesoDespachado->id << " | Prioridad: " << procesoDespachado->priority << endl;
    
    popMemoria();
    delete procesoDespachado;
}

void visualizarColaActual() {
    cout << "\n=============================================" << endl;
    cout << "     FILA DE ESPERA DE LA CPU DEL DRON       " << endl;
    cout << "=============================================" << endl;
    
    if (colaCPU == NULL) {
        cout << "-> Planificador vacío. No hay tareas en cola." << endl;
        return;
    }
    
    NodoProceso* temp = colaCPU;
    int posicion = 1;
    while (temp != NULL) {
        string descPrioridad = (temp->priority == 3) ? "ALTA (Emergencia)" : 
                               (temp->priority == 2) ? "MEDIA (Operación)" : "BAJA (Rutina)";
                               
        cout << posicion << ". [" << descPrioridad << "] Tarea: " << temp->nombre 
             << " (ID: " << temp->id << " | RAM: " << temp->memoria << " MB)" << endl;
        temp = temp->siguiente;
        posicion++;
    }
    cout << "=============================================" << endl;
}

void eliminarDeColaCpuPorId(int id) {
    if (colaCPU == NULL) return;
    
    if (colaCPU->id == id) {
        NodoProceso* aBorrar = colaCPU;
        colaCPU = colaCPU->siguiente;
        delete aBorrar;
        return;
    }
    
    NodoProceso* temp = colaCPU;
    while (temp->siguiente != NULL) {
        if (temp->siguiente->id == id) {
            NodoProceso* aBorrar = temp->siguiente;
            temp->siguiente = temp->siguiente->siguiente;
            delete aBorrar;
            return;
        }
        temp = temp->siguiente;
    }
}

// ============================================================================
// COMPONENTE 1: GESTOR DE PROCESOS (LISTA ENLAZADA SIMPLE)
// ============================================================================

void insertarProcesoLista(int id, string nombre, int prioridad, int memoria) {
    NodoProceso* temp = listaPrincipal;
    while (temp != NULL) {
        if (temp->id == id) {
            cout << "[Error] El ID " << id << " ya está asignado a otra tarea activa." << endl;
            return;
        }
        temp = temp->siguiente;
    }
    
    NodoProceso* nuevo = new NodoProceso();
    nuevo->id = id;
    nuevo->nombre = nombre;
    nuevo->priority = prioridad;
    nuevo->memoria = memoria;
    
    nuevo->siguiente = listaPrincipal;
    listaPrincipal = nuevo;
    
    pushMemoria(id, memoria);                     
    encolarPorPrioridad(id, nombre, prioridad, memoria); 
    
    cout << "[REGISTRO] Tarea '" << nombre << "' agregada con éxito al sistema." << endl;
}

void buscarProceso(string criterio, bool esId) {
    if (listaPrincipal == NULL) {
        cout << "[Historial] El registro base de procesos está vacío." << endl;
        return;
    }
    
    NodoProceso* temp = listaPrincipal;
    bool encontrado = false;
    
    while (temp != NULL) {
        if ((esId && convertirEnteroAString(temp->id) == criterio) || (!esId && temp->nombre == criterio)) {
            string descPrioridad = (temp->priority == 3) ? "ALTA (Emergencia)" : 
                                   (temp->priority == 2) ? "MEDIA (Operación)" : "BAJA (Rutina)";
            cout << "\n--- Proceso Localizado ---" << endl;
            cout << "ID: " << temp->id << endl;
            cout << "Nombre: " << temp->nombre << endl;
            cout << "Prioridad: " << descPrioridad << endl;
            cout << "Memoria RAM: " << temp->memoria << " MB" << endl;
            encontrado = true;
            break;
        }
        temp = temp->siguiente;
    }
    if (!encontrado) {
        cout << "No se encontró ningún proceso que coincida con el criterio de búsqueda." << endl;
    }
}

void modificarPrioridadDeProceso(int idBuscar, int nuevaPrioridad) {
    NodoProceso* temp = listaPrincipal;
    bool encontrado = false;
    string nombreProceso = "";
    int memoriaProceso = 0;
    
    while (temp != NULL) {
        if (temp->id == idBuscar) {
            temp->priority = nuevaPrioridad;
            nombreProceso = temp->nombre;
            memoriaProceso = temp->memoria;
            encontrado = true;
            break;
        }
        temp = temp->siguiente;
    }
    
    if (encontrado) {
        eliminarDeColaCpuPorId(idBuscar);
        encolarPorPrioridad(idBuscar, nombreProceso, nuevaPrioridad, memoriaProceso);
        cout << "[SISTEMA] Prioridad del proceso ID " << idBuscar << " modificada. Cola de CPU reorganizada." << endl;
    } else {
        cout << "[Error] El ID solicitado no está registrado en el dron." << endl;
    }
}

void eliminarProcesoLista(int idBuscar) {
    NodoProceso* actual = listaPrincipal;
    NodoProceso* anterior = NULL;
    bool encontrado = false;
    
    while (actual != NULL) {
        if (actual->id == idBuscar) {
            encontrado = true;
            break;
        }
        anterior = actual;
        actual = actual->siguiente;
    }
    
    if (encontrado) {
        if (anterior == NULL) {
            listaPrincipal = listaPrincipal->siguiente;
        } else {
            anterior->siguiente = actual->siguiente;
        }
        
        eliminarDeColaCpuPorId(idBuscar);
        
        cout << "[SISTEMA] Proceso ID " << idBuscar << " eliminado del sistema." << endl;
        delete actual;
    } else {
        cout << "[Error] No se encontró el proceso con ID " << idBuscar << " en el registro." << endl;
    }
}

void mostrarInventarioGlobal() {
    cout << "\n=============================================" << endl;
    cout << "       INVENTARIO GENERAL DE PROCESOS        " << endl;
    cout << "=============================================" << endl;
    if (listaPrincipal == NULL) {
        cout << "-> No hay procesos registrados en el historial general." << endl;
        return;
    }
    NodoProceso* temp = listaPrincipal;
    while (temp != NULL) {
        cout << "[ID: " << temp->id << "] | " << temp->nombre 
             << " | Prio: " << temp->priority << " | RAM: " << temp->memoria << " MB" << endl;
        temp = temp->siguiente;
    }
    cout << "=============================================" << endl;
}

// ============================================================================
// MENU INTERACTIVO DE USUARIO
// ============================================================================
int main() {
    int opcion;
    
    insertarProcesoLista(101, "Enviar_Telemetria", 1, 12);
    insertarProcesoLista(102, "Evasion_Obstaculo", 3, 32);
    insertarProcesoLista(103, "Estabilizar_Motores", 2, 20);

    do {
        cout << "\n=======================================================" << endl;
        cout << "   SISTEMA OPERATIVO EXPERIMENTAL - DRON DE RESCATE   " << endl;
        cout << "=======================================================" << endl;
        cout << "1. [GESTOR] Registrar Nueva Tarea (Insercion)" << endl;
        cout << "2. [GESTOR] Eliminar / Abortar Tarea" << endl;
        cout << "3. [GESTOR] Buscar Tarea por ID o Nombre" << endl;
        cout << "4. [GESTOR] Cambiar Prioridad de una Tarea" << endl;
        cout << "5. [PLANIFICADOR] Visualizar Cola de Prioridad del CPU" << endl;
        cout << "6. [PLANIFICADOR] Despachar y Ejecutar Siguiente Tarea Critica" << endl;
        cout << "7. [MEMORIA] Verificar Estado Actual del Stack de RAM" << endl;
        cout << "8. [VER] Mostrar Inventario Historico Completo" << endl;
        cout << "9. Salir de la Simulacion de Vuelo" << endl;
        cout << "Seleccione una opcion (1-9): ";
        cin >> opcion;

        switch(opcion) {
            case 1: {
                int id, prio, mem;
                string nom;
                cout << "Ingrese ID de la tarea: "; cin >> id;
                cout << "Ingrese Nombre (sin espacios): "; cin >> nom;
                cout << "Ingrese Prioridad (3:Alta, 2:Media, 1:Baja): "; cin >> prio;
                if (prio < 1 || prio > 3) {
                    cout << "Prioridad invalida. Operacion cancelada." << endl;
                    break;
                }
                cout << "Ingrese Memoria requerida (MB): "; cin >> mem;
                insertarProcesoLista(id, nom, prio, mem);
                break;
            }
            case 2: {
                int id;
                cout << "Ingrese el ID de la tarea a eliminar: "; cin >> id;
                eliminarProcesoLista(id);
                break;
            }
            case 3: {
                int subOpcion;
                cout << "Buscar por: 1) ID  2) Nombre: "; cin >> subOpcion;
                if (subOpcion == 1) {
                    string idB; cout << "Ingrese ID: "; cin >> idB;
                    buscarProceso(idB, true);
                } else {
                    string nomB; cout << "Ingrese Nombre: "; cin >> nomB;
                    buscarProceso(nomB, false);
                }
                break;
            }
            case 4: {
                int id, nuevaPrio;
                cout << "Ingrese ID de la tarea a modificar: "; cin >> id;
                cout << "Ingrese Nueva Prioridad (3:Alta, 2:Media, 1:Baja): "; cin >> nuevaPrio;
                if (nuevaPrio < 1 || nuevaPrio > 3) {
                    cout << "Prioridad invalida." << endl;
                    break;
                }
                modificarPrioridadDeProceso(id, nuevaPrio);
                break;
            }
            case 5:
                visualizarColaActual();
                break;
            case 6:
                desencolarCPU();
                break;
            case 7:
                verificarEstadoMemoria();
                break;
            case 8:
                mostrarInventarioGlobal();
                break;
            case 9:
                cout << "Simulacion finalizada. Descargando telemetria y apagando dron..." << endl;
                break;
            default:
                cout << "Opcion no valida. Intente de nuevo." << endl;
        }
    } while(opcion != 9);

    return 0;
}
