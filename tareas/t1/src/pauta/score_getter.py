'''
IIC2333 2019-1 - Sistemas Operativos y Redes - Corrector de T1
Autor: Germán Leandro Contreras Sagredo
Descripción: El siguiente script obtiene el puntaje de la tarea de un/a
alumno/a, a partir de la pauta y del output generado por este/esta.
'''

# Librerías a utilizar.
import pandas as pd      # Lectura de archivos CSV y revisión de valores.
from pathlib import Path # Existencia de archivos.
import sys               # Lectura de argumentos de la consola.

# Constantes a utilizar.
NP_TESTS = 5
P_TESTS = 5
NP_SCORE = 2
P_SCORE = 2

# Ejecución por consola.
if __name__ == "__main__":
    # El primer argumento corresponde a la carpeta del alumno/a.
    # El segundo argumento corresponde a la carpeta con las soluciones.
    student_path = sys.argv[1]
    corrector_path = sys.argv[2]
    # Puntaje por parte.
    scores = [0,0]
    # Ponderación por test
    test_scores = [NP_SCORE/NP_TESTS,P_SCORE/P_TESTS]
    # Revisamos cada test.
    for i in range(10):
        # Rutas del resultado y la pauta.
        result_path = f"out{i}.csv"
        solution_path = f"sol{i}.csv"
        # Índice que indica si es test es non-preemptive o preemptive.
        test_ind = i % 2
        # Primero, verificamos que la tarea haya generado el output.
        # Si el archivo existe, se revisa.
        result_file = Path(f"{student_path}/{result_path}")
        if result_file.is_file():
            # Usamos try-except para el caso de los outputs vacíos o mal
            # escritos.
            try:
                # Se leen los archivos.
                result_values = pd.read_csv(f"{student_path}/{result_path}")
                solution_values = pd.read_csv(f"{corrector_path}/{solution_path}")
                # Primero se comparan las dimensiones.
                # Número de procesos y de estadisticas.
                processes, stats = solution_values.shape
                # Advertencia 1 - Menos procesos de los que deberían.
                if result_values.shape[0] < processes:
                    print(f"Test {i} result has less processes than expected.")
                # Advertencia 2 - Menos estadisticas de las que deberían.
                if result_values.shape[1] < stats:
                    print(f"Test {i} result has less stats than expected.")
                # Si existen más estadísticas o más procesos, simplemente no se
                # consideran.
                # Advertencia 3 - Procesos no siguen el mismo orden. En este caso,
                # no se corrige el test.
                stop = False
                for p in range(processes):
                    # Si no son iguales, entonces el orden de los procesos NO ES
                    # CORRECTO, o bien los nombres no son correctos.
                    result_row = result_values.iloc[p]
                    solution_row = solution_values.iloc[p]
                    if result_row[0].strip() != solution_row[0]:
                        print(f"Test {i} processes order is WRONG or names are incorrect. Test {i}: 0 pts.")
                        stop = True
                        break
                # Si la condición se cumple, se pasa a la siguiente iteración.
                if stop:
                    continue
                # Procedemos a comparar los valores, en caso de que no nos hayamos
                # detenido.
                total = processes*(stats-1)
                points = 0
                for p in range(processes):
                    result_row = result_values.iloc[p]
                    solution_row = solution_values.iloc[p]
                    for s in range(1,stats):
                        # Si son iguales, aumenta el puntaje.
                        if (result_row[s] == solution_row[s]):
                            points += 1
                # Sumamos el puntaje total y redondeamos con dos decimales por
                # simplicidad.
                percent = round(points/total,2)
                score_percent = percent*test_scores[test_ind]
                scores[test_ind] += score_percent
                print(f"Test {i}: {points}/{total} = {score_percent} pts.")
            # El flujo anterior falló, por lo que se asume mala escritura del
            # archivo.
            except:
                print(f"Test {i} CSV file is empty or with an incorrect format. Test {i}: 0 pts.")
        # En otro caso, hubo un problema al generar el archivo. Se asignan 0
        # puntos.
        else:
            print(f"File {result_path} was not generated. Test {i}: 0 pts.")
    # Se imprimen los resultados finales.
    np_score, p_score = scores
    print(f"Score Non-preemptive: {np_score}\nScore Preemptive: {p_score}")
