# IIC2333 - Sistemas Operativos y Redes - Tests T1

## Ayudantes: Germán Leandro Contreras Sagredo, Ricardo Esteban Schilling Broussaingaray

### Tests para la revisión del _scheduler_.

Para la revisión de esta tarea se ejecutaron cinco _tests_ -con dos combinaciones de _input_ distintas para cada uno- ubicados en la carpeta **tests**:

1. **test1.txt:** _Test_ simple donde se revisa la ejecución básica de su programa. Evalúa la atención de procesos por prioridad.

2. **test2.txt:** _Test_ con un grado de dificultad mayor que evalúa la asignación correcta de la CPU después de las interrupciones.

3. **test3.txt:** _Test_ que evalúa simplemente la ejecución correcta del algoritmo para muchos procesos concurrentes, además de considerar el caso borde en el que el proceso es interrumpido al mismo tiempo que termina su ráfaga.

4. **test4.txt:** _Test_ correspondiente al subido como ejemplo. Evalúa los puntos (1) y (2) anteriores y uno adicional: el caso borde en el que el proceso interrumpido es el de mayor prioridad.

5. **test5.txt:** _Test_ que evalúa el siguiente caso borde: desempate de procesos a partir del `PID`.

Podrán notar que: 

1. Estos son de baja extensión y no suponen un mayor desafío de procesamiento. Esto, dado que el objetivo es corroborar que funcione lo más importante de forma correcta.

2. Si bien algunos evalúan casos borde, las estadísticas en las que influyen muchas veces son de un porcentaje bajo con respecto al _test_ -ejemplo: *test3.txt*-.

A continuación, los _outputs_ generados según las siguientes combinaciones de _test_ e _input_:

1. **_test 0_ - out0.csv:** `./scheduler test1.txt out0.csv np`

2. **_test 1_ - out1.csv:** `./scheduler test1.txt out1.csv p`

3. **_test 2_ - out2.csv:** `./scheduler test2.txt out2.csv np 4`

4. **_test 3_ - out3.csv:** `./scheduler test2.txt out3.csv p 4`

5. **_test 4_ - out4.csv:** `./scheduler test3.txt out4.csv np`

6. **_test 5_ - out5.csv:** `./scheduler test3.txt out5.csv p 10`

7. **_test 6_ - out6.csv:** `./scheduler test4.txt out6.csv np 10`

8. **_test 7_ - out7.csv:** `./scheduler test4.txt out7.csv p 3`

9. **_test 8_ - out8.csv:** `./scheduler test5.txt out8.csv np`

10. **_test 9_ - out9.csv:** `./scheduler test5.txt out9.csv p 4`

Notar que algunas combinaciones usan parámetros irrelevantes (por ejemplo, _tests_ no expropiativos con _quantum_ o bien indicar _q_ igual a 3 cuando es el estándar) solo para corroborar que lleven a cabo un uso adecuado de los parámetros.

Pueden, finalmente, ver los resultados de sus _tests_ haciendo uso del script `score_getter.py`. Este requiere hacer uso de `Python 3.6` con la librería `pandas` instalada y se ejecuta desde consola como sigue:

```
py -3.6 score_getter.py /path/to/username/T1 tests
``` 

En este caso, se asume que `/path/to/username/T1` posee todos los archivos CSV generados por su programa según el detalle anterior.