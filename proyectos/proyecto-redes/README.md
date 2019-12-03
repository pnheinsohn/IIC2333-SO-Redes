# proyecto-redes
Proyecto de redes para el ramo IIC2333

## Integrantes

- Paul Heinsohn: 1562305J
- Manuel Aguirre: 15624005
- Jacques Hasard: 1562188J
- Felipe Dominguez: 1562157J

## Referencias

- El logger se obtuvo del siguiente [link](http://simplestcodings.blogspot.com/2010/10/simple-logger-in-c.html)

## Principales decisiones de diseño
* Para las funciones del servidor y cliente, las principales funciones se encuentran en dentro de handlers.c. En este modulo, podemos apreciar que se encuentra la funcion encargada de manejar los casos de llegada de mensajes, para generar la respuesta adecuada segun sea canveniente.
* Por otro lado, podemos observar como el servidor es el encargado de almacenar la información del juego y de los clientes. Es decir, los clientes solo se encargan de enviar la información que necesiten para poder jugar, mientras que el servidor se encarga de la "logica" del juego y de las comunicaciones entre los clientes, lo cual se puede apreciar al ver `server/handler.c` y el uso de 
`Board`.
* Los tableros se encuentran en la estructura `Board`, la cual es instanciada dentro de `Game` para dar origen al tablero que sera usado durante el juego. Así, mediante backtraking nos fue posible implementar una funcion que fuera la encargada de revisar los movimientos y validar estos. En base a lo siguiente, es posible afirmar que todas las acciones correspondientes al juego (movimiento de fichas, ganadores, etc...) se encuentran en `board.c`.


## Supuestos
* Podemos asumir que el mensaje maximo a enviar (entre clientes) es de 100 caracteres.
* Cuando realizamos un disconnect desde cualquiera de los jugadores (y obviamente desde el servidor), desconectamos a ambos clientes. Así, un cliente quiere volver a jugar, es necesario que se conecte nuevamente.
* Asumimos que el ip y puerto ingresados por el cliente y servidor son validos.

