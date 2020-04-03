# Progetto del Corso *Programmazione di Sistema* A.A. 18/19

La disponibilità di connessioni a larga banda e l’esigenza di lavorare in gruppo senza richiedere necessariamente la compresenza degli attori nello stesso spazio fisico, spinge verso la realizzazione di sistemi di supporto al lavoro cooperativo sempre più efficaci. Ad esempio, Google mette a disposizione la suite Docs, mediante la quale è possibile editare, in modo cooperativo e distribuito, documenti di varia natura (testi, fogli di calcolo, presentazioni) ed in grado di scalare su numeri anche grandi di utenti contemporanei: tale soluzione è basata su un insieme di server centralizzati che gestiscono il traffico da e verso i singoli client e mettono in atto la logica necessaria a garantire la correttezza delle operazioni concorrenti.

Quello che abbiamo realizzato, utilizzando il linguaggio C++, è un sistema di editing testuale cooperativo che consenta a uno o più utenti di modificare il contenuto di un documento in contemporanea, garantendo che operazioni di inserimento o modifica diverse, svolte dagli utenti allo stesso tempo, producano gli stessi effetti, indipendentemente dall’ordine con cui sono eseguite sui diversi sistemi in uso (commutatività) e che cancellazioni ripetute portino allo stesso risultato (idempotenza).

# Lato Client
Il progetto si divide in due aree principali, client e server, in cui vengono gestite rispettivamente la comunicazione front-end (quindi anche la parte di grafica) e quella back-end.

Lato client sono presenti diversi file .cpp, ciascuno con associato file header e interfaccia grafica.

## welcome.cpp
Questa è la pagina di apertura dell'applicazione. Da qui è possibile scegliere di registrarsi o, se già si possiede un account, di accedervi.

## login.cpp
Nel caso in cui si clicchi sul bottone "Login" della pagina precedente, si è condotti a questa schermata, in cui è richiesto l'inserimento di username e password. Dopo averne verificato la correttezza lato server, l'utente viene reindirizzato alla pagina di apertura del documento.

## signup.cpp
In questa pagina all'utente viene chiesto di registrarsi inserendo username, password e conferma della password. E' anche possibile scegliere un'icona, nel caso in cui non venga fatto ne verrà assegnata una in automatico con l'iniziale dell'utente.

## newopen.cpp
Questa è la schermata intermedia a cui si arriva dopo aver fatto il login. Vi è una lista di file presenti nel filesystem a cui è possibile accedere. E' anche possibile inserire l'URI di un documento a cui l'utente è stato invitato a collaborare: se il link è corretto il file apparirà tra quelli selezionabili nella lista a sinistra e potrà essere aperto. Infine l'utente può anche decidere di creare un nuovo file.

## mainwindow.cpp
Il vero e proprio editor di testo. 
Nel menu in evidenza ci sono le operazioni principali: creazione di un nuovo file, apertura file, undo, redo, le varie funzioni di arricchimento del testo (grassetto, corsivo, sottolineato, allineamento) e infine il logout. 
Ben visibile è anche l'URI del documento corrente, per un eventuale invito a collaborare.
In alto a destra c'è l'icona dell'utente connesso e, man mano che altri utenti si aprono lo stesso documento, compaiono anche le loro icone (fino a un massimo di tre utenti, dopo di che i restanti vengono aggiunti ad un contatore che, se cliccato, posta i dettagli degli altri utenti).

In questo file viene gestita tutta la parte della concorrenza a livello grafico, legate alla modifica contemporanea dello stesso documento. 

In particolare:

 - `on_textEdit_textChanged():` invia i segnali che comunicano l'inserimento o la cancellazione di una lettera.
 - `changeViewAfterInsert(QChar  l,  int  pos,  QTextCharFormat  format):` riceve come parametri la nuova lettera da inserire, la sua posizione e il suo formato, per visualizzarla sul client corrente.
 - `changeViewAfterDelete(int  pos)`: riceve come parametro la posizione in cui cancellare la lettera.
 - `on_cursor_triggered(QPair<int,int>  idpos,  QColor  col):`questa funzione viene scatenata nel momento in cui uno degli altri utenti connessi al documento cambia la posizione del cursore (i cursori vengono visualizzati evidenziando la lettera alla loro sinistra del colore identificativo dell'utente).  Le corrispondenze tra utenti, colori e posizione corrente del cursore sono memorizzate in una lista indicizzata con gli id degli utenti, *id_colore_cursore*, che viene aggiornata ogni volta che il sucrose di un utente cambia la sua posizione.
 - `on_actionExport_as_PDF_triggered():`questa funzione permette di esportare il documento in formato PDF.

## account.cpp
L'utente ha una sua pagina personale, accessibile dalla pagina principale, in cui può modificare sia la sua icona che la sua password.
