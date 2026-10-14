#ifndef PROTOCOLO_JUEGO_H
#define PROTOCOLO_JUEGO_H

#include <string>

// Puerto y host por defecto para el juego por sockets
#define DEFAULT_GAME_HOST "127.0.0.1"
#define DEFAULT_GAME_PORT 8080

// Comandos de comunicación
namespace Protocolo {
    const std::string CMD_CONECTAR   = "CONECTAR";
    const std::string CMD_TIRAR      = "TIRAR";
    const std::string CMD_SALIR      = "SALIR";

    const std::string RESP_INFO      = "INFO";
    const std::string RESP_ASIGNADO  = "ASIGNADO";
    const std::string RESP_INICIO    = "INICIO";
    const std::string RESP_TURNO     = "TURNO";
    const std::string RESP_TABLERO   = "TABLERO";
    const std::string RESP_JUGADA    = "JUGADA";
    const std::string RESP_FIN       = "FIN";
    const std::string RESP_ERROR     = "ERROR";
}

#endif // PROTOCOLO_JUEGO_H
