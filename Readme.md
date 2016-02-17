## Rete di Sensori - Descrizione
Una rete di molteplici sensori emette sequenze di misurazioni.
I client sono interessati alle misurazioni emesse da un sotto-insieme dei sensori ed è necessario realizzare il servizio
che permette ai client di ricevere solamente la fusione delle misurazioni dei sensori a cui si sono dichiarati interessati.

### Ulteriori Specifiche
I sensori hanno una connessione persistente con il server, 
emettano misurazioni con bassa frequenza - non superiore ai 10hz (penso a sensori ambientali di temperatura, umidità luminosità...),
che i client abbiamo una connessione persistente 
e che il feed ricevuto sia real-time - il server non si occupa di agglomerare più eventi insieme, 
ma li ripropone immediatamente a tutti i client interessati al sensore.

## Dettagli tecnologici
Il progetto è diviso in due parti, una libreria Thread Safe `"TSAR"` (Thread SAfe Router) che smista i messaggi ricevuti in diverse code, una per ogni subscriber di eventi,
e un applicativo che espone il servizio su una porta attraverso un semplice protocollo testuale

### Multithreading
il linguaggio scelto è C11 su compilatore clang. questo permette di utilizzare gli header (threads.h)[http://en.cppreference.com/w/c/thread]
e (atomic.h)[http://en.cppreference.com/w/c/atomic] per utilizzare in maniera portabile le primitive per poter sfruttare la concorrenza.
Questi librerie standard sono ispirate da (pthread.h)[http://pubs.opengroup.org/onlinepubs/007908799/xsh/pthread.h.html], per cui sarebbe facile
convertire l'implementazione per sfruttare invece questa libreria.

#### Supporto della stdlib
Sfortunatamente glibc manca attualmente di una implementazione per `threads.h`. Sarebbe possibile includere un layer che implementa l'interfaccia con `pthread`,
ma visto che la semantica delle due api non coincide perfettamente, per questo progretto è utilizzato `pthread` direttamente.

### Testing
Per i test è utilizzato [catch](https://github.com/philsquared/Catch), un framework c++ per il BDD. Per i test verrà utilizzato c++14, con la libreria
opportunamente inclusa come `extern "C"`, mentre l'eseguibile sarà testato attraverso connessioni tcp su interfaccia di loopback

## Design 
La libreria è thread safe ma non multithread: le funzioni sono bloccanti o con timeout, e sta ai client implementare il multithread
L'applicativo invece è multithread: uno per leggere lo stream in ingresso dei dati, un thread di controllo ed un thread per ogni subscriber.

### tsar.h
l'header `tsar.h` espone le funzioni della libreria

La libreria vive nello slice di tempo fra due funzioni
    
    tsar_t* tsar_init();
    void tsar_destroy(tsar_t*);

la chiamata ritorna un puntatore a `tsar_t` che permette di manipolare l'istanza di router associata per ottenere lo stream di entrata
e uno o più stream di uscita

    in_chan_t* publisher_create(tsar_t* t, char* name, int name_l );
    void publisher_remove(in_chan_t*);
    
la prima funzione registra un Publisher e ritorna un `in_chan_t` - da usare come canale per trasmettere i associare ai propri messaggi.
**name** serve per permettere ai Subscribers di recuperare lo stream associato.
`in_chan_t` è legato al `name`, e le chiamate successive con lo stesso `name` ritornano NULL.
la seconda funzione chiude il canale e manda ai Subscriber una poison pill che li avverte della fine dello stream.

Creare e distruggere un publisher significa creare e distruggere una entry dentro una mappa che tiene traccia del canale associato ad ogni publisher,
e che viene utilizzata sia per garantire l'unicità del canale, sia per creare i canali di lettura dei Subscribers.
Abbiamo quindi da una parte operazioni di lettura e di operazioni di scrittura su una struttura dati. 
a seconda di come la libreria è usata, potrebbe esserci una discrepanza fra il numero di letture e il numero di scritture.
per esempio, un sistema dove un monitor ascolta più sensori avrà più scritture - per il numero di canali creato - mentre un sistema dove
più ascoltatori ascoltano un numero ristretto di sensori avrà più letture - perché più ascoltatori dovranno accedere alla struttura dati per 
poter creare il proprio canale

Tale disequilibrio (in un senso o nell'altro) fra letture e scritture può giustificare un `pthread_rwlock`, per massimizzare il troughput. 
In pratica senza delle misurazioni è difficile calibrare la politica di locking, e in generale l'evento di creazione di canali è più raro, 
rispetto all'evento principale di distribuzione di un messaggio - che non tocca questa struttura dati. Per questo le operazioni che lavorano sulla mappa 
publisher->canale acquisiscono un unico mutex, che rende l'implementazione più semplice

### lib/inmemory_logger.h
ispirato da [http://preshing.com/20120522/lightweight-in-memory-logging/](http://preshing.com/20120522/lightweight-in-memory-logging/)
questo header contiene un logger che espone una macro

    LOG(const char* message, long param)
    
attivata nelle build di debug. 

il log è scritto in un buffer circolare di grandezza **65536**, e per influire il meno possibile sull'esecuzione la struttura è lock-free.
Ogni thread riceve lo slot in cui scrivere incrementando una variabile globale atomica, e usando il valore come indice di un array globale (modulo la grandezza).
Questo non garantisce che due thread non possano interferire fra di loro, ma la grandezza del buffer rende una interferenza improbabile, nel dominio attuale del problema.
Inoltre l'evento non è catastrofico, perché nessuna operazione dipende da una lettura della struttura dati.

l'assembly generato con `-O2` mostra la semplicità dell'operazione, che il compilatore è propenso a porre inline
    
    ; Function Attrs: nounwind uwtable
    define void @inmem_log(i8* %msg, i64 %param) #0 {
      %1 = atomicrmw add i64* @inmem_pos, i64 1 seq_cst
      %2 = and i64 %1, 65535
      %3 = tail call i64 @pthread_self() #2
      %4 = getelementptr inbounds [65536 x %struct.inmem_event]* @inmem_buffer, i64 0, i64 %2, i32 0
      store i64 %3, i64* %4, align 16, !tbaa !1
      %5 = getelementptr inbounds [65536 x %struct.inmem_event]* @inmem_buffer, i64 0, i64 %2, i32 1
      store i8* %msg, i8** %5, align 8, !tbaa !7
      %6 = getelementptr inbounds [65536 x %struct.inmem_event]* @inmem_buffer, i64 0, i64 %2, i32 2
      store i64 %param, i64* %6, align 16, !tbaa !8
      ret void
    }
