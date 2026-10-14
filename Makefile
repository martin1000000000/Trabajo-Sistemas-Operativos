# ─── Makefile SistOpe (Entrega 3) ───────────────────────────
CXX      = g++
CXXFLAGS = -Wall -std=c++17 -Iinclude -pthread
LDFLAGS  = -lws2_32

# 1. Programa Principal SistOpe (con indice invertido paralelo)
SRC_MAIN    = src/main.cpp src/utilidades.cpp src/usuarios.cpp src/perfiles.cpp src/indice.cpp
TARGET_MAIN = SistOpe

# 2. Subproceso Multiplicador de Matrices
SRC_MULTI    = src/multiplicador.cpp
TARGET_MULTI = multiplicador

# 3. Servidor de Juego Multiplayer (Sockets)
SRC_SERVER    = src/servidor_juego.cpp src/utilidades.cpp
TARGET_SERVER = servidor_juego

# 4. Cliente de Juego Multiplayer (Sockets)
SRC_CLIENT    = src/cliente_juego.cpp src/utilidades.cpp
TARGET_CLIENT = cliente_juego

# Compilar todo
all: $(TARGET_MAIN) $(TARGET_MULTI) $(TARGET_SERVER) $(TARGET_CLIENT)

$(TARGET_MAIN): $(SRC_MAIN)
	$(CXX) $(CXXFLAGS) $(SRC_MAIN) -o $(TARGET_MAIN) $(LDFLAGS)

$(TARGET_MULTI): $(SRC_MULTI)
	$(CXX) $(CXXFLAGS) $(SRC_MULTI) -o $(TARGET_MULTI)

$(TARGET_SERVER): $(SRC_SERVER)
	$(CXX) $(CXXFLAGS) $(SRC_SERVER) -o $(TARGET_SERVER) $(LDFLAGS)

$(TARGET_CLIENT): $(SRC_CLIENT)
	$(CXX) $(CXXFLAGS) $(SRC_CLIENT) -o $(TARGET_CLIENT) $(LDFLAGS)

# Compilar y ejecutar programa principal
run: $(TARGET_MAIN)
	./$(TARGET_MAIN) -u PorDefecto -p 123 -f El_Mundo_de_Sofia.txt

# Limpiar ejecutables
ifeq ($(OS),Windows_NT)
clean:
	-cmd /c "del /Q /F $(TARGET_MAIN).exe $(TARGET_MULTI).exe $(TARGET_SERVER).exe $(TARGET_CLIENT).exe 2>nul"
else
clean:
	-rm -f $(TARGET_MAIN) $(TARGET_MAIN).exe $(TARGET_MULTI) $(TARGET_MULTI).exe $(TARGET_SERVER) $(TARGET_SERVER).exe $(TARGET_CLIENT) $(TARGET_CLIENT).exe
endif

.PHONY: all run clean
