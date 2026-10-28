#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <thread>
#include <mutex>
#include <memory>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <random>
#include <algorithm>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
typedef int socklen_t;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
typedef int SOCKET;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define closesocket close
#endif

#include "protocolo_juego.h"
#include "utilidades.h"

using namespace std;

// ─── Estructura de Jugador Conectado ─────────────────────
struct Jugador {
    SOCKET socket;
    string nombre;
    string equipo; // "EquipoA" o "EquipoB"
    bool activo;
};

// ─── Estado Global del Servidor ──────────────────────────
static mutex g_mutexJuego;
static mutex g_mutexLog;

static vector<shared_ptr<Jugador>> g_jugadores;
static int g_posEquipoA = 0;
static int g_posEquipoB = 0;
static int g_metaTablero = 0;
static int g_carasDado = 0;
static bool g_partidaIniciada = false;
static bool g_partidaFinalizada = false;

static string g_archivoLog = "";
static mt19937 g_rng(static_cast<unsigned int>(chrono::system_clock::now().time_since_epoch().count()));

// ─── Función de Registro de Log ──────────────────────────
// Formato requerido: YYYY-MM-DD HH:MM:SS;jugador;equipo;ACCION;detalle
static void registrarLog(const string& jugador, const string& equipo, const string& accion, const string& detalle = "") {
    lock_guard<mutex> lock(g_mutexLog);

    auto ahora = chrono::system_clock::now();
    time_t tiempoT = chrono::system_clock::to_time_t(ahora);
    tm tmInfo;
#ifdef _WIN32
    localtime_s(&tmInfo, &tiempoT);
#else
    localtime_r(&tiempoT, &tmInfo);
#endif

    stringstream ssFecha;
    ssFecha << put_time(&tmInfo, "%Y-%m-%d %H:%M:%S");

    string lineaLog = ssFecha.str() + ";" + jugador + ";" + equipo + ";" + accion;
    if (!detalle.empty()) {
        lineaLog += ";" + detalle;
    }

    // Mostrar en consola del servidor
    cout << "[LOG] " << lineaLog << endl;

    // Persistir en archivo de log
    ofstream fLog(g_archivoLog, ios::app);
    if (fLog.is_open()) {
        fLog << lineaLog << "\n";
        fLog.flush();
        fLog.close();
    }
}

// ─── Envío de Mensajes a Clientes ────────────────────────
static void enviarMensaje(SOCKET sock, const string& mensaje) {
    string msgCompleto = mensaje + "\n";
    send(sock, msgCompleto.c_str(), static_cast<int>(msgCompleto.length()), 0);
}

static void broadcast(const string& mensaje) {
    for (const auto& j : g_jugadores) {
        if (j->activo) {
            enviarMensaje(j->socket, mensaje);
        }
    }
}

// ─── Control de Turnos por Equipos (Round-Robin Alternado) ───
static shared_ptr<Jugador> g_jugadorEnTurno = nullptr;
static string g_proximoEquipo = "EquipoA";
static size_t g_idxRotacionA = 0;
static size_t g_idxRotacionB = 0;

static vector<shared_ptr<Jugador>> obtenerActivosEquipo(const string& equipo) {
    vector<shared_ptr<Jugador>> res;
    for (const auto& j : g_jugadores) {
        if (j->activo && j->equipo == equipo) {
            res.push_back(j);
        }
    }
    return res;
}

static shared_ptr<Jugador> obtenerSiguienteTurno() {
    auto activosA = obtenerActivosEquipo("EquipoA");
    auto activosB = obtenerActivosEquipo("EquipoB");

    if (activosA.empty() && activosB.empty()) {
        g_jugadorEnTurno = nullptr;
        return nullptr;
    }

    // Determinar qué equipo debe jugar (alternando, a menos que un equipo no tenga activos)
    string equipoElegido = g_proximoEquipo;
    if (equipoElegido == "EquipoA" && activosA.empty()) {
        equipoElegido = "EquipoB";
    } else if (equipoElegido == "EquipoB" && activosB.empty()) {
        equipoElegido = "EquipoA";
    }

    if (equipoElegido == "EquipoA") {
        if (g_idxRotacionA >= activosA.size()) g_idxRotacionA = 0;
        g_jugadorEnTurno = activosA[g_idxRotacionA];
        g_idxRotacionA = (g_idxRotacionA + 1) % activosA.size();
        g_proximoEquipo = "EquipoB";
    } else {
        if (g_idxRotacionB >= activosB.size()) g_idxRotacionB = 0;
        g_jugadorEnTurno = activosB[g_idxRotacionB];
        g_idxRotacionB = (g_idxRotacionB + 1) % activosB.size();
        g_proximoEquipo = "EquipoA";
    }

    return g_jugadorEnTurno;
}

static shared_ptr<Jugador> obtenerTurnoActual() {
    if (!g_jugadorEnTurno || !g_jugadorEnTurno->activo) {
        return obtenerSiguienteTurno();
    }
    return g_jugadorEnTurno;
}

// ─── Notificar estado del juego ──────────────────────────
static void notificarTurnoYTablero() {
    auto jTurno = obtenerTurnoActual();
    if (!jTurno) return;

    // Enviar posiciones actuales del tablero
    broadcast("TABLERO|" + to_string(g_posEquipoA) + "|" + to_string(g_posEquipoB) + "|" + to_string(g_metaTablero));
    
    // Anunciar turno
    broadcast("TURNO|" + jTurno->nombre + "|" + jTurno->equipo);
    registrarLog(jTurno->nombre, jTurno->equipo, "CAMBIO_TURNO");
}

// ─── Verificar condición de inicio ───────────────────────
// Condición: al menos 2 equipos y cada equipo con al menos 2 jugadores (mínimo 4)
static void verificarCondicionInicio() {
    if (g_partidaIniciada) return;

    int cantA = 0;
    int cantB = 0;
    for (const auto& j : g_jugadores) {
        if (j->activo) {
            if (j->equipo == "EquipoA") cantA++;
            else if (j->equipo == "EquipoB") cantB++;
        }
    }

    if (cantA >= 2 && cantB >= 2) {
        g_partidaIniciada = true;
        cout << "\n==========================================" << endl;
        cout << "  [SERVIDOR] CONDICION DE INICIO CUMPLIDA!" << endl;
        cout << "  Iniciando partida multiplayer..." << endl;
        cout << "==========================================" << endl;

        registrarLog("SISTEMA", "TODOS", "INICIO_PARTIDA", "Tablero=" + to_string(g_metaTablero) + ";Dado=" + to_string(g_carasDado));
        broadcast("INICIO|Comienza la partida! Meta: " + to_string(g_metaTablero) + " casillas.");
        
        g_proximoEquipo = "EquipoA";
        g_idxRotacionA = 0;
        g_idxRotacionB = 0;
        obtenerSiguienteTurno();
        notificarTurnoYTablero();
    } else {
        string estadoEspera = "Esperando jugadores para iniciar... EquipoA: " + to_string(cantA) + "/2 | EquipoB: " + to_string(cantB) + "/2";
        cout << "[SERVIDOR] " << estadoEspera << endl;
        broadcast("INFO|" + estadoEspera);
    }
}

// ─── Hilo de atención por cliente ────────────────────────
static void atenderCliente(SOCKET sockCliente) {
    shared_ptr<Jugador> jugadorActual = nullptr;
    char buffer[1024];

    while (true) {
        int bytesRecv = recv(sockCliente, buffer, sizeof(buffer) - 1, 0);
        if (bytesRecv <= 0) {
            // Desconexión detectada
            break;
        }

        buffer[bytesRecv] = '\0';
        stringstream ss(buffer);
        string linea;

        while (getline(ss, linea)) {
            while (!linea.empty() && (linea.back() == '\r' || linea.back() == '\n')) {
                linea.pop_back();
            }
            if (linea.empty()) continue;

            // Procesar comando del cliente
            stringstream ssCmd(linea);
            string comando;
            getline(ssCmd, comando, '|');

            if (comando == Protocolo::CMD_CONECTAR) {
                string nombre, equipoDeseado;
                getline(ssCmd, nombre, '|');
                getline(ssCmd, equipoDeseado, '|');

                if (nombre.empty()) nombre = "Jugador_" + to_string(sockCliente);

                lock_guard<mutex> lock(g_mutexJuego);

                // Si el nombre ya existe (ej. varios clientes con 'PorDefecto'), asignar sufijo único
                string nombreBase = nombre;
                int sufijo = 2;
                while (true) {
                    bool existe = false;
                    for (const auto& j : g_jugadores) {
                        if (j->activo && j->nombre == nombre) {
                            existe = true;
                            break;
                        }
                    }
                    if (!existe) break;
                    nombre = nombreBase + "_" + to_string(sufijo++);
                }

                // Asignar equipo
                int conteoA = 0, conteoB = 0;
                for (const auto& j : g_jugadores) {
                    if (j->activo) {
                        if (j->equipo == "EquipoA") conteoA++;
                        else conteoB++;
                    }
                }

                string equipoAsignado = equipoDeseado;
                if (equipoAsignado != "EquipoA" && equipoAsignado != "EquipoB") {
                    equipoAsignado = (conteoA <= conteoB) ? "EquipoA" : "EquipoB";
                } else if (equipoAsignado == "EquipoA" && conteoA >= 4) {
                    equipoAsignado = "EquipoB";
                } else if (equipoAsignado == "EquipoB" && conteoB >= 4) {
                    equipoAsignado = "EquipoA";
                }

                jugadorActual = make_shared<Jugador>();
                jugadorActual->socket = sockCliente;
                jugadorActual->nombre = nombre;
                jugadorActual->equipo = equipoAsignado;
                jugadorActual->activo = true;

                g_jugadores.push_back(jugadorActual);

                registrarLog(nombre, equipoAsignado, "CONECTA");
                registrarLog(nombre, equipoAsignado, "ASIGNA_EQUIPO", equipoAsignado);

                enviarMensaje(sockCliente, "ASIGNADO|" + nombre + "|" + equipoAsignado);
                broadcast("INFO|El jugador '" + nombre + "' se unio a " + equipoAsignado + ".");

                verificarCondicionInicio();

            } else if (comando == Protocolo::CMD_TIRAR) {
                lock_guard<mutex> lock(g_mutexJuego);

                if (!g_partidaIniciada) {
                    enviarMensaje(sockCliente, "ERROR|La partida aun no ha comenzado. Esperando jugadores.");
                    continue;
                }
                if (g_partidaFinalizada) {
                    enviarMensaje(sockCliente, "ERROR|La partida ya ha finalizado.");
                    continue;
                }

                auto jTurno = obtenerTurnoActual();
                if (!jTurno || jTurno->socket != sockCliente) {
                    enviarMensaje(sockCliente, "ERROR|No es tu turno de lanzar el dado.");
                    continue;
                }

                // Tirar dado de R caras
                uniform_int_distribution<int> dist(1, g_carasDado);
                int dado = dist(g_rng);

                registrarLog(jTurno->nombre, jTurno->equipo, "LANZA_DADO", to_string(dado));

                int posAnterior = 0;
                int posNueva = 0;

                if (jTurno->equipo == "EquipoA") {
                    posAnterior = g_posEquipoA;
                    g_posEquipoA += dado;
                    posNueva = g_posEquipoA;
                } else {
                    posAnterior = g_posEquipoB;
                    g_posEquipoB += dado;
                    posNueva = g_posEquipoB;
                }

                registrarLog(jTurno->nombre, jTurno->equipo, "AVANZA", to_string(posAnterior) + ";" + to_string(posNueva));

                // Notificar jugada a todos
                broadcast("JUGADA|" + jTurno->nombre + "|" + jTurno->equipo + "|" + to_string(dado) + "|" + to_string(posAnterior) + "|" + to_string(posNueva));

                // Verificar término
                if (posNueva >= g_metaTablero) {
                    g_partidaFinalizada = true;
                    registrarLog(jTurno->nombre, jTurno->equipo, "GANA", "Posicion=" + to_string(posNueva));
                    registrarLog("SISTEMA", "TODOS", "FIN_PARTIDA", "Ganador=" + jTurno->equipo);

                    broadcast("FIN|" + jTurno->equipo + "|" + jTurno->equipo + " alcanzo la casilla " + to_string(posNueva) + " y GANA LA CARRERA!");
                } else {
                    // Pasar al siguiente turno
                    obtenerSiguienteTurno();
                    notificarTurnoYTablero();
                }

            } else if (comando == Protocolo::CMD_SALIR) {
                break;
            }
        }
    }

    // Manejo de desconexión del cliente
    {
        lock_guard<mutex> lock(g_mutexJuego);
        if (jugadorActual && jugadorActual->activo) {
            jugadorActual->activo = false;
            registrarLog(jugadorActual->nombre, jugadorActual->equipo, "DESCONECTA");
            broadcast("INFO|El jugador '" + jugadorActual->nombre + "' se ha desconectado.");

            // Si estaba en turno, avanzar al siguiente
            if (g_partidaIniciada && !g_partidaFinalizada && g_jugadorEnTurno == jugadorActual) {
                obtenerSiguienteTurno();
                notificarTurnoYTablero();
            }
        }
    }

    closesocket(sockCliente);
}

// ─── Main del Servidor ───────────────────────────────────
int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    cout << "==========================================" << endl;
    cout << "  SERVIDOR DE JUEGO MULTIPLAYER (SOCKETS)" << endl;
    cout << "==========================================" << endl;

    // 1. Leer y validar variables de entorno (sin nombres ni valores hardcodeados)
    g_archivoLog = obtenerVariableEnvOConfig("GAME_LOG");
    if (g_archivoLog.empty()) {
        cout << "[ERROR] La variable de entorno GAME_LOG no esta definida. Abortando." << endl;
        return 1;
    }

    int puerto = 0;
    int metaX = 0;
    int carasR = 0;
    if (!validarVariableEntero("GAME_PORT", puerto, 1024, 65535) ||
        !validarVariableEntero("TABLERO_X", metaX, 10, 1000) ||
        !validarVariableEntero("DADO_R", carasR, 2, 100)) {
        cout << "[ERROR] Variables de entorno del juego invalidas o faltantes. Abortando." << endl;
        return 1;
    }
    g_metaTablero = metaX;
    g_carasDado = carasR;

    cout << "[CONFIG] Archivo de Log: " << g_archivoLog << endl;
    cout << "[CONFIG] Puerto TCP    : " << puerto << endl;
    cout << "[CONFIG] Meta Tablero  : " << g_metaTablero << " casillas" << endl;
    cout << "[CONFIG] Caras del Dado: " << g_carasDado << endl;
    cout << "------------------------------------------" << endl;

#ifdef _WIN32
    // 2. Inicializar Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "[ERROR] Fallo al inicializar Winsock." << endl;
        return 1;
    }
#endif

    // 3. Crear socket de escucha
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        cout << "[ERROR] No se pudo crear socket servidor." << endl;
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    // Reutilización de dirección
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(static_cast<uint16_t>(puerto));

    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        cout << "[ERROR] Fallo al enlazar socket al puerto " << puerto << "." << endl;
        closesocket(serverSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        cout << "[ERROR] Fallo en listen." << endl;
        closesocket(serverSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    cout << "[INFO] Servidor activo escuchando en puerto " << puerto << "..." << endl;
    cout << "[INFO] Esperando conexiones de clientes. Presione Ctrl+C para finalizar.\n" << endl;

    // 4. Bucle principal de aceptación de clientes
    vector<thread> hilosClientes;

    while (true) {
        sockaddr_in clientAddr{};
        socklen_t clientSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, reinterpret_cast<sockaddr*>(&clientAddr), &clientSize);

        if (clientSocket == INVALID_SOCKET) {
            cout << "[AVISO] Error al aceptar cliente entrante." << endl;
            continue;
        }

        char ipCliente[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), ipCliente, INET_ADDRSTRLEN);
        cout << "[CONEXION] Nuevo cliente conectado desde " << ipCliente << ":" << ntohs(clientAddr.sin_port) << endl;

        // Lanzar hilo independiente por cliente
        hilosClientes.emplace_back(atenderCliente, clientSocket);
    }

    // Cierre controlado de recursos
    closesocket(serverSocket);
    for (auto& t : hilosClientes) {
        if (t.joinable()) t.join();
    }
#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
