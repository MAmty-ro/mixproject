🎛️ CD-DA Custom DAW & Burner - Documentazione Ufficiale

Questo progetto è una Digital Audio Workstation (DAW) minimale e specializzata, progettata per estrarre tracce audio dal web, manipolarle su una timeline virtuale con dissolvenze incrociate (crossfade), e masterizzarle direttamente su supporti fisici CD-R rispettando rigorosamente lo standard internazionale Red Book (CD-DA).
🟥 1. Vincoli di Sistema & Specifiche Hardware (Red Book)

Per far sì che il CD finale sia leggibile da qualsiasi autoradio o lettore CD degli anni '90, il software è vincolato ai parametri hardware del formato IEC 908 (Standard Red Book):

    Formato Audio Richiesto: PCM Lineare non compresso.

    Frequenza di Campionamento: Rigorosamente 44100 Hz (l'orologio hardware dei convertitori CD).

    Risoluzione: 16-bit per campione.

    Canali: 2 canali (Stereo).

    Capacità Massima del Supporto: 74 minuti (standard) o 80 minuti (700 MB).

    Dimensione dei Blocchi (Settori): Ogni settore sul CD fisico contiene esattamente 2352 byte di dati audio grezzi.

📁 2. Architettura del File System (Workspace)

Il progetto è strutturato per separare nettamente la logica di backend, l'interfaccia grafica e i file temporanei generati durante il rendering:

mixproject/
├── main.cpp                 # Entry point dell'applicazione (Inizializzazione Qt)
├── DOCUMENTATION.md         # Questo file
├── session_logs.txt         # Registro storico degli errori e dei download
├── include/                 # File Header (.h) - Dichiarazione delle classi
│   ├── FinestraPrincipale.h # Interfaccia grafica principale (Qt6)
│   ├── TracciaAudio.h       # Struttura dati e parser del singolo file WAV
│   ├── MixerTimeline.h      # Struttura dati della griglia temporale e mixdown
│   └── MasterizzatoreCD.h   # Gestione dell'hardware di masterizzazione
├── src/                     # Codice Sorgente (.cpp) - Logica di basso livello
│   ├── FinestraPrincipale.cpp # Gestione eventi GUI, download e aggiornamento liste
│   ├── TracciaAudio.cpp     # Parsing binario, lettura header ed estrazione campioni
│   ├── MixerTimeline.cpp    # Motore di mixdown e hard-limiter
│   └── MasterizzatoreCD.cpp # Interfaccia con i comandi di sistema per CD-R
├── scripts/                 # Pipeline esterne (Python / Shell)
│   └── downloader.py        # Estrattore audio da URL remoti (yt-dlp / ffmpeg)
└── build/                   # Directory di output per l'eseguibile compilato

🎨 3. Interfaccia Grafica (Tracklist View)

L'interfaccia utente abbandona la complessità di una griglia a timeline orizzontale in favore di una Tracklist Lineare Architettata, ottimizzata per la creazione sequenziale di CD Audio.

    Palette Colori (Cyberpunk High-Contrast): Sfondi Nero Profondo (#0A0A0A), pannelli di controllo Viola Elettrico (#4B0082), indicatori e cursori Rosa Shocking (#FF1493), attivazioni hardware Rosso Laser (#FF0000).

    Funzionalità Interfaccia:

        Input immediato degli URL con barra di progresso Rosa dinamica.

        Visualizzazione a moduli verticali per le tracce in coda, con calcolo automatico e visualizzazione della durata reale estratta dall'header del file binario.

        Monitor in tempo reale del minutaggio totale del CD rispetto al limite dei 74 minuti del Red Book.

        🛠️ 4. Cheat Sheet dei Comandi (Terminale Fedora)
Avvio manuale del Downloader (Script isolato):

python3 scripts/downloader.py "URL_DI_YOUTUBE"

Compilazione Totale del Progetto (GUI + Motore Audio):

g++ -o build/daw_engine main.cpp src/TracciaAudio.cpp src/MixerTimeline.cpp src/MasterizzatoreCD.cpp src/FinestraPrincipale.cpp -I include/ `pkg-config --cflags --libs Qt6Widgets` -fPIC

Esecuzione della DAW:

./build/daw_engine

📈 5. Stato del Progetto & Roadmap

    [x] Fase 1: Sviluppo dello script Python per il download e la conversione automatica in WAV (44100Hz, Stereo, 16-bit).

    [x] Fase 2: Parsing dell'header binario del file WAV in C++ ed estrazione della durata effettiva.

    [x] Fase 3: Creazione della GUI Cyberpunk con Qt6 Widgets e integrazione della pipeline di download a tempo reale.

    [ ] Fase 4: Sviluppo del modulo di masterizzazione su CD-R (Interfaccia nativa C++ con strumenti di sistema come wodim / cdrskin).

    [ ] Fase 5: Implementazione del motore di mixdown con calcolo dinamico dei Crossfade (dissolvenze incrociate) tra le tracce in lista.

    ### 🎛️ Aggiornamento Modulo di Trasporto & Gestione Timeline (v2.0)

L'interfaccia grafica e il motore DSP sono stati potenziati con un controllo temporale lineare in stile YouTube e algoritmi predittivi per il monitoraggio dei punti di giunzione.

#### 1. Barra di Avanzamento Interattiva (Timeline Slider)
* **Rendering Sincrono:** Al click su `PLAY`, il motore effettua il mix DSP in background generandone l'anteprima hardware. Un oscillatore hardware (`QTimer` con polling a 100ms) interroga la testina del `QMediaPlayer` aggiornando la posizione dello slider fucsia/ciano.
* **Scribbling Manuale (Seek):** È supportato il trascinamento manuale della barra. Il sistema disattiva temporaneamente i segnali di aggiornamento automatico durante la pressione (`isUserScribbling = true`) per evitare conflitti di clock e sposta la riproduzione hardware all'esatto millisecondo rilasciato.
* **Minutaggio Dinamico:** Visualizzazione costante del counter temporale formattato in formato `MM:SS / MM:SS` (Tempo corrente / Durata totale del mix).

#### 2. Logica di Protezione Traccia #1 (Inizio CD)
* Per garantire la conformità agli standard del Red Book Audio (CD-DA), la **Traccia #1** agisce da punto di ancoraggio della Timeline a $0.0\text{s}$.
* Il selettore di transizione (`QComboBox`) viene **completamente rimosso** dal primo slot grafico e sostituito da un blocco statico `🏁 INIZIO CD`. Le transizioni e i relativi crossfade (5 secondi) diventano attivi e configurabili esclusivamente a partire dalla seconda traccia in poi.

#### 3. Algoritmo di Salto Intelligente (`⏭️ SALTA A -10s TRANS.`)
Invece di un salto statico, il pulsante esegue una scansione predittiva in tempo reale basata sulla posizione attuale della testina audio:
1. Cattura il timestamp corrente del flusso PipeWire.
2. Scansiona ricorsivamente la Timeline calcolando i punti di intersezione reali (tenendo conto dei 5 secondi di sovrapposizione dei Crossfade inseriti).
3. Identifica la prima transizione futura rispetto alla posizione attuale.
4. Sposta istantaneamente la testina del player **esattamente 10 secondi prima dell'inizio del mix** tra i due brani, permettendo un ascolto immediato della bontà del crossfade senza dover riprodurre l'intera traccia.