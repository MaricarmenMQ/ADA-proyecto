# Detective Algorítmico — Caso Dinámico

Proyecto de Análisis y Diseño de Algoritmos. Resuelve un caso policial cruzando:

- **Grafo de coartadas** (lista de adyacencia) + **DFS** para detectar ciclos ("anillos de coartadas")
  y componentes conexas — *estructura de datos avanzada / grafos*.
- **Backtracking con poda** (CSP: variables booleanas culpable/inocente, codificadas como bitmask) para
  encontrar la asignación consistente con menos culpables — *backtracking de proceso complejo*.
- **Simulación de Monte Carlo** sobre la credibilidad de cada testigo — *algoritmo probabilístico*.
- **QuickSort** para armar el ranking final por probabilidad de culpabilidad.

El caso **no es estático**: no hay ningún arreglo fijo de sospechosos "quemado" en el algoritmo.
Tanto la consola en C++ como la interfaz web permiten construir el caso desde cero (agregar/quitar
sospechosos y testimonios) además de traer un caso de ejemplo para arrancar rápido.

## Estructura de carpetas

```
Detective-CrimenLogico/
│
├── src/                       ← código C++, un algoritmo por archivo
│   ├── main.cpp                orquesta el flujo, menú de consola
│   ├── sospechoso.h             struct Sospechoso + struct Testimonio
│   ├── caso.h                   clase Caso: alta/baja dinámica, cargar/guardar archivo
│   ├── grafo_coartadas.h        lista de adyacencia, dfsCiclos(), componentesConexas()
│   ├── backtracking.h           clase Backtracking: CSP con bitmask + poda
│   ├── montecarlo.h             simulacionCulpabilidad()
│   └── ordenamiento.h           quicksortRanking()
│
├── interfaz/                  ← visualización web (HTML/CSS/JS), no reemplaza la consola
│   ├── index.html
│   ├── styles.css
│   └── app.js
│
└── README.md                  ← este archivo
```

## Compilar y ejecutar (C++)

Requiere g++ con soporte C++17.

```bash
cd src
g++ -std=c++17 -Wall -O2 main.cpp -o detective
./detective            # menú interactivo
./detective --auto     # corre todo en secuencia con el caso de ejemplo, útil para grabar demo
```

### Menú de consola

```
 1. Ver sospechosos y testimonios cargados
 2. Agregar un sospechoso
 3. Agregar un testimonio
 4. Nuevo caso en blanco (armar tu propia historia)
 5. Cargar un caso predefinido de ejemplo (hay 3)
 6. Cargar caso desde archivo .txt
 7. Guardar caso actual en archivo .txt
 8. Construir y mostrar grafo de coartadas
 9. Detectar ciclos en el grafo (DFS)
10. Mostrar componentes conexas (DFS)
11. Resolver combinacion de culpables (Backtracking + poda)
12. Simular culpabilidad (Monte Carlo)
13. Mostrar ranking final (QuickSort)
14. Ver analisis de complejidad consolidado
15. Ejecutar TODO en secuencia (modo expo)
 0. Salir
```

La opción 5 deja elegir entre **3 casos de ejemplo** distintos (los mismos "Hacienda" y "Puerto"
de la interfaz web, más el original "Vagón Azul"), y la opción 4 arranca un **caso completamente
en blanco** (sin nada precargado) para construir tu propia historia con las opciones 2 y 3. Nada
queda "quemado" en el algoritmo: puedes tener cualquiera de los 3 ejemplos, uno propio armado a
mano, o uno cargado desde un archivo `.txt` externo.

Formato de archivo de caso (opción 6 y 7), por línea:
```
PERSONA;nombre;credibilidad
TESTIMONIO;idTestigo;idAcusado;esCoartada(0/1);declaracion
```

## Interfaz web

Abre `interfaz/index.html` con doble clic (no necesita servidor ni dependencias). Reproduce
exactamente la misma lógica que el C++ (mismo esquema de bitmask, misma condición de consistencia,
mismo criterio de Monte Carlo y de QuickSort), con una interfaz tipo "expediente de investigación":

- Barra lateral con selector de expedientes (puedes tener varios casos a la vez y crear nuevos con
  "+ Nuevo expediente") y un pipeline de 6 pasos que se marca en vivo.
- Botón "▶ Ejecutar investigación" que corre todo el flujo en secuencia (grafo → DFS → backtracking
  → Monte Carlo → QuickSort) con una consola (`detective_algoritmico.log`) que va narrando cada paso.
- Tablero de sospechosos como grafo interactivo (avatares con iniciales, aristas de coartada,
  flechas de acusación) que resalta ciclos de DFS y culpables del CSP.
- Sección de gestión de expediente para agregar/eliminar sospechosos y testimonios en vivo; nada
  está precargado de forma fija, todo el análisis se recalcula sobre lo que se ingresa ahí.

## Análisis de complejidad (resumen — ver el informe para el detalle)

| Algoritmo | Mejor caso | Caso promedio | Peor caso |
|---|---|---|---|
| DFS (ciclos / componentes) | O(V+E) | O(V+E) | O(V+E) |
| Backtracking (CSP, n sospechosos, m testimonios) | O(n·m) | O(2^(n/2)·m) *(empírico)* | O(2ⁿ·m) |
| Monte Carlo (S simulaciones) | O(S·(n+m)) | O(S·(n+m)) | O(S·(n+m)) |
| QuickSort (n sospechosos) | O(n log n) | O(n log n) | O(n²) |

n está acotado a 24 sospechosos (`MAX_SOSPECHOSOS` en `sospechoso.h`) para que el espacio de
búsqueda 2ⁿ del backtracking siga siendo manejable en una demo en vivo.
