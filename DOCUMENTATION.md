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