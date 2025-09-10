PARA EMBEBIDOS, SE DEBE DEFINIR:
#define _USE_STDLIB
o definir esta constante desde las opciones de keil.
Esto es para indicarle al compilador que se debe usar stdlib para obtener ciertos tipos como size_t. Caso contrario, se intenta buscar una libreria disponible en los sistemas operativos.