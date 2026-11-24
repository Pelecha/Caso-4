set(NOMBRE_EJECUTABLE ferrocarril)

add_executable(${NOMBRE_EJECUTABLE}
    main.cpp
    Linea.h
    Linea.cpp
    Tramo.h
    Tramo.cpp
    Estacion.h
    Estacion.cpp
)
target_link_libraries(${NOMBRE_EJECUTABLE} PRIVATE base semaforo)
