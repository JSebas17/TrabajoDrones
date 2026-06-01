#include <iostream>
#include <string>
#include <sstream> // Necesario para to_string alternativo en C++ antiguo
#include <fstream> // Agrega esto para leer y escribir archivos

using namespace std;
const int MAX_PROCESOS = 100;

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
    cout << "      ESTADO DE LA MEMORIA RAM DEL DRON      " << endl;
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

void aplicarBurbujaEnCola() {
    if (colaCPU == NULL || colaCPU->siguiente == NULL) return;
    bool intercambiado;
    NodoProceso* actual;
    NodoProceso* ultimoCambiado = NULL;
    do {
        intercambiado = false;
        actual = colaCPU;
        while (actual->siguiente != ultimoCambiado) {
            if (actual->priority < actual->siguiente->priority) {
                // Intercambio de valores internos usando Burbuja
                int tempId = actual->id;
                string tempNombre = actual->nombre;
                int tempPriority = actual->priority;
                int tempMemoria = actual->memoria;

                actual->id = actual->siguiente->id;
                actual->nombre = actual->siguiente->nombre;
                actual->priority = actual->siguiente->priority;
                actual->memoria = actual->siguiente->memoria;

                actual->siguiente->id = tempId;
                actual->siguiente->nombre = tempNombre;
                actual->siguiente->priority = tempPriority;
                actual->siguiente->memoria = tempMemoria;
                intercambiado = true;
            }
            actual = actual->siguiente;
        }
        ultimoCambiado = actual;
    } while (intercambiado);
}

void encolarPorPrioridad(int id, string nombre, int prioridad, int memoria) {
    NodoProceso* nuevoNodo = new NodoProceso();
    nuevoNodo->id = id;
    nuevoNodo->nombre = nombre;
    nuevoNodo->priority = prioridad;
    nuevoNodo->memoria = memoria;
    nuevoNodo->siguiente = NULL;
    
    if (colaCPU == NULL) {
        colaCPU = nuevoNodo;
    } else {
        NodoProceso* temp = colaCPU;
        while (temp->siguiente != NULL) {
            temp = temp->siguiente;
        }
        temp->siguiente = nuevoNodo;
    }
    aplicarBurbujaEnCola(); // Aplica ordenamiento burbuja aquí
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

void buscarProcesoPorBinaria(int idBuscar) {
    if (listaPrincipal == NULL) {
        cout << "[Historial] El registro base de procesos esta vacio." << endl;
        return;
    }
    NodoProceso* arregloAux[MAX_PROCESOS];
    int tamano = 0;
    NodoProceso* temp = listaPrincipal;
    while (temp != NULL && tamano < MAX_PROCESOS) {
        arregloAux[tamano] = temp;
        tamano++;
        temp = temp->siguiente;
    }
    // Ordena el arreglo temporal con burbuja para poder hacer la binaria
    for (int i = 0; i < tamano - 1; i++) {
        for (int j = 0; j < tamano - i - 1; j++) {
            if (arregloAux[j]->id > arregloAux[j+1]->id) {
                NodoProceso* t = arregloAux[j];
                arregloAux[j] = arregloAux[j+1];
                arregloAux[j+1] = t;
            }
        }
    }
    // ALGORITMO DE BÚSQUEDA BINARIA
    int izquierda = 0;
    int derecha = tamano - 1;
    int posicionEncontrada = -1;
    while (izquierda <= derecha) {
        int medio = izquierda + (derecha - izquierda) / 2;
        if (arregloAux[medio]->id == idBuscar) {
            posicionEncontrada = medio;
            break;
        }
        if (arregloAux[medio]->id < idBuscar) izquierda = medio + 1;
        else derecha = medio - 1;
    }
    if (posicionEncontrada != -1) {
        NodoProceso* encontrado = arregloAux[posicionEncontrada];
        string descPrioridad = (encontrado->priority == 3) ? "ALTA (Emergencia)" : 
                               (encontrado->priority == 2) ? "MEDIA (Operacion)" : "BAJA (Rutina)";
        cout << "\n--- Proceso Localizado (Mediante Busqueda Binaria) ---" << endl;
        cout << "ID: " << encontrado->id << endl;
        cout << "Nombre: " << encontrado->nombre << endl;
        cout << "Prioridad: " << descPrioridad << endl;
        cout << "Memoria RAM: " << encontrado->memoria << " MB" << endl;
    } else {
        cout << "No se encontro ningun proceso con el ID " << idBuscar << " usando Busqueda Binaria." << endl;
    }
}

void buscarProcesoPorNombreSecuencial(string nombreBuscar) {
    NodoProceso* temp = listaPrincipal;
    bool encontrado = false;
    while (temp != NULL) {
        if (temp->nombre == nombreBuscar) {
            string descPrioridad = (temp->priority == 3) ? "ALTA (Emergencia)" : 
                                   (temp->priority == 2) ? "MEDIA (Operacion)" : "BAJA (Rutina)";
            cout << "\n--- Proceso Localizado (Secuencial por Nombre) ---" << endl;
            cout << "ID: " << temp->id << endl;
            cout << "Nombre: " << temp->nombre << endl;
            cout << "Prioridad: " << descPrioridad << endl;
            cout << "Memoria RAM: " << temp->memoria << " MB" << endl;
            encontrado = true;
            break;
        }
        temp = temp->siguiente;
    }
    if (!encontrado) cout << "No se encontro la tarea con el nombre: " << nombreBuscar << endl;
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
    cout << "       INVENTARIO GENERAL DE PROCESOS        " << endl;
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

// 1. FUNCIÓN PARA GUARDAR LOS DATOS EN UN ARCHIVO TXT
void guardarDatosEnArchivo() {
    // Abrimos el archivo en modo escritura (output file stream)
    ofstream archivo("procesos_dron.txt");
    
    if (!archivo.is_open()) {
        cout << "[ERROR] No se pudo abrir el archivo para guardar los datos." << endl;
        return;
    }
    
    NodoProceso* temp = listaPrincipal;
    // Recorremos la lista guardando un proceso por cada línea, separado por comas
    while (temp != NULL) {
        archivo << temp->id << ","
                << temp->nombre << ","
                << temp->priority << ","
                << temp->memoria << "\n";
        temp = temp->siguiente;
    }
    
    archivo.close();
}

// 2. FUNCIÓN PARA CARGAR LOS DATOS DESDE EL ARCHIVO TXT
// VERSION COMPATIBLE DE CARGA DE DATOS (SIN USAR STOI)
void cargarDatosDesdeArchivo() {
    ifstream archivo("procesos_dron.txt");
    
    if (!archivo.is_open()) {
        cout << "[SISTEMA] No se encontro un respaldo previo. Iniciando sistema limpio." << endl;
        return;
    }
    
    int id, prio, mem;
    string nom;
    string linea;
    
    while (getline(archivo, linea)) {
        if (linea.empty()) continue;
        
        stringstream ss(linea);
        string token;
        
        // Usamos variables stream intermedias para transformar texto a entero de forma clasica
        getline(ss, token, ','); 
        stringstream convId(token); convId >> id;
        
        getline(ss, nom, ',');
        
        getline(ss, token, ','); 
        stringstream convPrio(token); convPrio >> prio;
        
        getline(ss, token, ','); 
        stringstream convMem(token); convMem >> mem;
        
        // Insertamos en el sistema de manera segura
        insertarProcesoLista(id, nom, prio, mem);
    }
    
    archivo.close();
}


int main() {
    int opcion;
    
    cargarDatosDesdeArchivo();

    do {
        cout << "\n=======================================================" << endl;
        cout << "   SISTEMA OPERATIVO EXPERIMENTAL - DRON DE RESCATE   " << endl;
        cout << "=======================================================" << endl;
        cout << "1. Registrar Nueva Tarea (Insercion)" << endl;
        cout << "2. Eliminar / Abortar Tarea" << endl;
        cout << "3. Buscar Tarea por ID o Nombre" << endl;
        cout << "4. Cambiar Prioridad de una Tarea" << endl;
        cout << "5. Visualizar Cola de Prioridad del CPU" << endl;
        cout << "6. Despachar y Ejecutar Siguiente Tarea Critica" << endl;
        cout << "7. Verificar Estado Actual del Stack de RAM" << endl;
        cout << "8. Mostrar Inventario Historico Completo" << endl;
        cout << "9. Salir de la Simulacion de Vuelo" << endl;
        cout << "Seleccione una opcion (1-9): ";
        
        if (!(cin >> opcion)) { 
            cout << "\n[ALERTA] Entrada invalida. Por favor, ingrese un NUMERO entre 1 y 9." << endl;
            cin.clear(); // Resetea el estado de error de cin
            cin.ignore(10000, '\n'); // Descarta las letras basura que causan el bucle
            opcion = 0; // Asigna un valor neutro para forzar a que repita el bucle limpiamente
            continue; 
        }
        
        switch(opcion) {
            case 1: {
                int id, prio, mem;
                string nom;
                
                // Validación para el ID
                cout << "Ingrese ID de la tarea: "; 
                while (!(cin >> id)) {
                    cout << "[Error] El ID debe ser un numero entero. Intente de nuevo: ";
                    cin.clear();
                    cin.ignore(10000, '\n');
                }
                
                cout << "Ingrese Nombre (sin espacios): "; 
                cin >> nom;
                
                // Validación para la Prioridad
                cout << "Ingrese Prioridad (3:Alta, 2:Media, 1:Baja): "; 
                while (!(cin >> prio) || prio < 1 || prio > 3) {
                    cout << "[Error] Prioridad invalida. Debe ser 1, 2 o 3. Intente de nuevo: ";
                    cin.clear();
                    cin.ignore(10000, '\n');
                }
                
                // Validación para la Memoria
                cout << "Ingrese Memoria requerida (MB): "; 
                while (!(cin >> mem) || mem <= 0) {
                    cout << "[Error] La cantidad de memoria debe ser un numero positivo. Intente de nuevo: ";
                    cin.clear();
                    cin.ignore(10000, '\n');
                }
                
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
                cout << "Buscar por: 1) ID (Busqueda Binaria)  2) Nombre (Secuencial): "; cin >> subOpcion;
                if (subOpcion == 1) {
                    int idB; cout << "Ingrese ID numerico a buscar: "; cin >> idB;
                    buscarProcesoPorBinaria(idB); // Llama a la binaria
                } else {
                    string nomB; cout << "Ingrese Nombre exacto: "; cin >> nomB;
                    buscarProcesoPorNombreSecuencial(nomB);
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
                guardarDatosEnArchivo(); 
                cout << "Simulacion finalizada. Apagando dron..." << endl;
                break;
            default:
                cout << "Opcion no valida. Intente de nuevo." << endl;
        }
    } while(opcion != 9);

    return 0;
}
