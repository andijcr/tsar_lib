# Phoenix
## *poor man build server*

`init.sh` crea un server virtuale Docker contenente `clang-3.6` e `cmake`, collega la directory del codice sorgente ad esso,
ed esegue al suo interno `build.sh`

a sua volta `build.sh` esegue un build attraverso cmake, ed esegue i test specificati al suo interno.

questa coppia di script è utile per verificare la compilazione su un sistema non alterato