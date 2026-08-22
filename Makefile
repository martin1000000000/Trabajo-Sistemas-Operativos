# ─── Makefile SistOpe ───────────────────────────────────────
CXX      = g++
CXXFLAGS = -Wall -std=c++17 -Iinclude
SRC      = src/main.cpp src/utilidades.cpp src/usuarios.cpp src/perfiles.cpp
TARGET   = SistOpe

# Compilar
all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

# Compilar y ejecutar
run: $(TARGET)
	./$(TARGET)

# Limpiar ejecutable
clean:
	rm -f $(TARGET) $(TARGET).exe

.PHONY: all run clean
