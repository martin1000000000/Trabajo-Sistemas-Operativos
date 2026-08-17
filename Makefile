# ─── Makefile SistOpe ───────────────────────────────────────
CXX      = g++
CXXFLAGS = -Wall -std=c++17
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
	del /Q $(TARGET).exe 2>nul || rm -f $(TARGET)

.PHONY: all run clean
