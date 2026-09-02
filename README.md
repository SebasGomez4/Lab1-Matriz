# Lab1-Matriz

En el laboratorio se nos plantea como problema la creación de una matriz de 100.000 x 100.000 en el disco duro, se tiene como objetivo solucionar los siguientes problemas:
-Consumo Excesivo de Ram
-Escritura lenta a disco
- Optimización en la manipulación, creación, almacenamiento y lectura de datos

# Elección alternativa de solución:
Luego de evaluar e indagar diversas alternativas a la resolución del problema, se opto por la utilización de un archivo binario disperso en el lenguaje C, alternativa planteada dentro de un foro de programación (Stack Overflow)

# ¿Cómo se resuelve el problema?
Se crea un archivo de metadata del cual se extraen las dimensiones y tamaño de cortes que permite la creación de un archivo binario disperso de 37.2 GB que permite leerse y cortarse de forma adecuada y cumpliendo los requisitos sin importar el programa o sistema externo.

Hacemos uso de la orden "fsctl_set_sparse" en la cual llenaremos todo el espacio con ceros virtuales vacios, sin necesidad de ocupar las 37.2 GB físicas de ceros, solo asegurandonos de que con el "_fseeki64" saltemos hasta el último elemento de nuestra matriz y llenemos el resto con la orden ya mencionada, además esto permite manejar una complejidad constante de O(1) lo que facilita la lectura y almacenamiento de los datos.

El archivo evitara el problema de consumo de RAM ya que asignara un espacio virtual lógico en lugar de un espacio meramente físico para el espacio que requiere la creación de la matriz (columnas x bytes por elemento); Además al crear la matriz usando floats (4 bytes/32 bits) cualquier consulta del usuario, leera unicamente el espacio reservado para el elemento.

# ¿Como se verifica el contenido de la matriz?
Luego de generada la matriz y que se abre la aplicación de consola el usuario podrá interactuar con un menú en el que puede pedirle la cantidad de elementos de cualquier fila o columna, en este caso un contador comprobara la presencia de 100.000 elementos dentro de la fila o columna

# ¿Que archivos se suben?
Se adjuntara únicamente un archivo .c en 2 versiones, 1 exclusiva de windows (totalmente documentada) y otra compatible para cualquier sistema, al cual al compilar y ejecutar creara un archivo .meta, un archivo .bin y una aplicación de consola en la que se podrá ver un menú interactivo

ADVERTENCIA:
Crear una carpeta dentro del disco local para compilar y ejecutar el archivo .c ya que si se corre dentro de documentos OneDrive intentara crear las 37.2 GB de manera física.
