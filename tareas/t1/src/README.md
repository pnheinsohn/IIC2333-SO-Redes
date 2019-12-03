# ReadMe Tarea 1

* Para compilar basta ejecutar "make"
* Para ejecutar basta con ejecutar "./scheduler <test_file.txt> <output.csv> \<version> [quantum]"
* Nótese que para revisar cómo ejecuta el programa hay varios printf's comentados en "structs.c" y "main.c", por lo que podría resultar útil descomentarlos
* En los tests de ejemplos tengo buenas las estadísticas excepto en el out3.txt (quantum=3) específicamente en el response_time y waiting_time de GERMY, en los que me paso por la unidad debido algún caso borde. Estimo que se debe a que su tiempo de llegada (!= 0) coincide con su entrada a RUNNING, pero podría estar errado.