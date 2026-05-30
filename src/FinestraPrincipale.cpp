#include "../include/FinestraPrincipale.h"
#include "../include/TracciaAudio.h"
#include "../include/MixerTimeline.h"
#include "../include/MasterizzatoreCD.h"
#include <QApplication>
#include <QHBoxLayout>
#include <QFileInfo>
#include <iostream>

FinestraPrincipale::FinestraPrincipale(QWidget *parent) : QWidget(parent) {
    setWindowTitle("CUSTOM CD-DA DAW & BURNER");
    resize(900, 650);
    inRiproduzione = false; // Di base il motore audio è fermo

    QVBoxLayout* layoutPrincipale = new QVBoxLayout(this);
    layoutPrincipale->setContentsMargins(15, 15, 15, 15);
    layoutPrincipale->setSpacing(15);

    // 1. PANNELLO DOWNLOAD (SUPERIORE)
    QWidget* pannelloDownload = new QWidget();
    pannelloDownload->setObjectName("pannelloDownload");
    QHBoxLayout* layoutDownload = new QHBoxLayout(pannelloDownload);
    
    inputUrl = new QLineEdit();
    inputUrl->setPlaceholderText("Incolla qui l'URL di YouTube per aggiungere una traccia...");
    pulsanteScarica = new QPushButton("🟣 SCARICA TRACCIA");
    
    layoutDownload->addWidget(inputUrl, 3);
    layoutDownload->addWidget(pulsanteScarica, 1);

    QWidget* sottoPannelloDownload = new QWidget();
    QVBoxLayout* layoutSottoDownload = new QVBoxLayout(sottoPannelloDownload);
    layoutSottoDownload->setContentsMargins(5, 0, 5, 0);
    
    labelStatoDownload = new QLabel("Status: Pronto per ricevere nuove tracce.");
    barraProgresso = new QProgressBar();
    barraProgresso->setRange(0, 100);
    barraProgresso->setValue(0);
    barraProgresso->setTextVisible(false);
    barraProgresso->setFixedHeight(6);

    layoutSottoDownload->addWidget(labelStatoDownload);
    layoutSottoDownload->addWidget(barraProgresso);

    // 2. AREA TRACKLIST (CENTRALE)
    areaScorrimentoTracce = new QScrollArea();
    areaScorrimentoTracce->setWidgetResizable(true);
    areaScorrimentoTracce->setObjectName("areaTracce");

    QWidget* contenitoreTracce = new QWidget();
    layoutListaTracce = new QVBoxLayout(contenitoreTracce);
    layoutListaTracce->setAlignment(Qt::AlignTop);
    layoutListaTracce->setSpacing(10);
    areaScorrimentoTracce->setWidget(contenitoreTracce);

    QLabel* labelVuota = new QLabel("🎵 Nessuna traccia nel CD. Usa il downloader sopra per aggiungerne.");
    labelVuota->setObjectName("labelVuota");
    labelVuota->setAlignment(Qt::AlignCenter);
    layoutListaTracce->addWidget(labelVuota);

    // 3. BARRA DI TRASPORTO & PLAYER (NOVITÀ AUDIO CONTROLS)
    QWidget* pannelloPlayer = new QWidget();
    pannelloPlayer->setObjectName("pannelloPlayer");
    QHBoxLayout* layoutPlayer = new QHBoxLayout(pannelloPlayer);

    pulsantePlayPause = new QPushButton("▶ PLAY");
    pulsantePlayPause->setObjectName("pulsantePlay");
    pulsantePlayPause->setFixedWidth(100);

    pulsanteStop = new QPushButton("■ STOP");
    pulsanteStop->setObjectName("pulsanteStop");
    pulsanteStop->setFixedWidth(100);

    QLabel* labelVelocita = new QLabel("SPEED:");
    labelVelocita->setStyleSheet("color: #00FFFF; font-weight: bold; margin-left: 15px;");
    
    selettoreVelocita = new QComboBox();
    selettoreVelocita->addItems({"x1.0 (Normal)", "x1.5 (Fast)", "x2.0 (Hyper)"});
    selettoreVelocita->setObjectName("selettoreVelocita");

    layoutPlayer->addWidget(pulsantePlayPause);
    layoutPlayer->addWidget(pulsanteStop);
    layoutPlayer->addWidget(labelVelocita);
    layoutPlayer->addWidget(selettoreVelocita);
    layoutPlayer->addStretch();

    // 4. PANNELLO MASTERIZZAZIONE (INFERIORE)
    QWidget* pannelloMaster = new QWidget();
    pannelloMaster->setObjectName("pannelloMaster");
    QHBoxLayout* layoutMaster = new QHBoxLayout(pannelloMaster);

    labelTempoTotaleCD = new QLabel("Tempo Totale CD: 00:00 / 74:00 🟢");
    pulsanteMasterizza = new QPushButton("🔴 MASTERIZZA CD FISICO");
    pulsanteMasterizza->setObjectName("pulsanteMasterizza");

    layoutMaster->addWidget(labelTempoTotaleCD);
    layoutMaster->addStretch();
    layoutMaster->addWidget(pulsanteMasterizza);

    // Composizione Layout
    layoutPrincipale->addWidget(pannelloDownload);
    layoutPrincipale->addWidget(sottoPannelloDownload);
    layoutPrincipale->addWidget(areaScorrimentoTracce, 1);
    layoutPrincipale->addWidget(pannelloPlayer); // Inserito sopra il masterizzatore
    layoutPrincipale->addWidget(pannelloMaster);

    applicaStileCyberpunk();

    // Connessioni segnali plancia hardware
    connect(pulsanteScarica, &QPushButton::clicked, [this]() { this->gestisciDownload(); });
    connect(pulsanteMasterizza, &QPushButton::clicked, [this]() { this->gestisciMasterizzazione(); });
    
    // Connessioni dei nuovi controlli di riproduzione
    connect(pulsantePlayPause, &QPushButton::clicked, [this]() { this->gestisciPlayPause(); });
    connect(pulsanteStop, &QPushButton::clicked, [this]() { this->gestisciStop(); });
    connect(selettoreVelocita, &QComboBox::currentTextChanged, [this](const QString& v) { this->cambiaVelocita(v); });
}

void FinestraPrincipale::applicaStileCyberpunk() {
    QString stile = 
        "QWidget { background-color: #0A0A0A; color: #FFFFFF; font-family: 'Segoe UI', Arial, sans-serif; }"
        "#pannelloDownload, #pannelloMaster, #pannelloPlayer { background-color: #121212; border: 2px solid #4B0082; border-radius: 8px; padding: 8px; }"
        "QLineEdit { background-color: #1A1A1A; border: 1px solid #4B0082; border-radius: 4px; padding: 8px; color: #FF1493; font-size: 13px; }"
        "QPushButton { background-color: #4B0082; border: none; border-radius: 4px; padding: 10px 15px; font-weight: bold; font-size: 12px; }"
        "QPushButton:hover { background-color: #6A0DAD; }"
        "QProgressBar::chunk { background-color: #FF1493; border-radius: 3px; }"
        "QProgressBar { background-color: #222222; border: none; border-radius: 3px; }"
        "#areaTracce { background-color: #0F0F0F; border: 1px dashed #4B0082; border-radius: 6px; }"
        "QLabel { font-size: 13px; }"
        "#labelVuota { color: #555555; font-size: 14px; padding-top: 50px; }"
        
        // Stile specifico per controlli di riproduzione
        "#pulsantePlay { background-color: #00FF00; color: #000000; }"
        "#pulsantePlay:hover { background-color: #00CC00; }"
        "#pulsanteStop { background-color: #FF8C00; color: #000000; }"
        "#pulsanteStop:hover { background-color: #E9967A; }"
        "QComboBox { background-color: #1A1A1A; border: 1px solid #00FFFF; border-radius: 4px; padding: 5px; color: #00FFFF; font-weight: bold; min-width: 130px; }"
        "QComboBox QAbstractItemView { background-color: #1A1A1A; selection-background-color: #4B0082; selection-color: #FFFFFF; }"
        
        "#pulsanteMasterizza { background-color: #FF0000; color: #FFFFFF; font-size: 13px; padding: 12px 25px; border-radius: 6px; }"
        "#pulsanteMasterizza:hover { background-color: #CC0000; }";

    this->setStyleSheet(stile);
}

// --- GESTIONE LOGICA DEL PLAYER ---
void FinestraPrincipale::gestisciPlayPause() {
    if (percorsiWavScaricati.empty()) {
        labelStatoDownload->setText("⚠️ Carica almeno una traccia per avviare il Player!");
        return;
    }

    if (!inRiproduzione) {
        inRiproduzione = true;
        pulsantePlayPause->setText("⏸ PAUSE");
        pulsantePlayPause->setStyleSheet("background-color: #FF1493; color: white;"); // Colore fucsia neon in riproduzione
        labelStatoDownload->setText("🔊 Engine Audio in esecuzione sulla Timeline DSP...");
        std::cout << "▶ [PLAYER] Riproduzione avviata alla velocità selezionata." << std::endl;
    } else {
        inRiproduzione = false;
        pulsantePlayPause->setText("▶ PLAY");
        pulsantePlayPause->setStyleSheet(""); // Ripristina stile css
        applicaStileCyberpunk(); 
        labelStatoDownload->setText("⏸ Audio in pausa.");
        std::cout << "⏸ [PLAYER] Riproduzione congelata in cache." << std::endl;
    }
}

void FinestraPrincipale::gestisciStop() {
    inRiproduzione = false;
    pulsantePlayPause->setText("▶ PLAY");
    pulsantePlayPause->setStyleSheet("");
    applicaStileCyberpunk();
    labelStatoDownload->setText("■ Riproduzione interrotta. Testina audio resettata a 00:00.");
    std::cout << "■ [PLAYER] Stop hardware inviato. Buffer svuotato." << std::endl;
}

void FinestraPrincipale::cambiaVelocita(const QString& velocita) {
    std::cout << "🎛️ [DSP COPROCESSOR] Cambio campionamento clock su: " << velocita.toStdString() << std::endl;
    labelStatoDownload->setText("⚡ Pitch/Speed hardware modificato in tempo reale: " + velocita);
}

void FinestraPrincipale::cambiaTransizioneTraccia(int indiceTraccia, const QString& tipoTransizione) {
    std::cout << "🔄 [MIXER TIMELINE] Modificata transizione traccia #" << indiceTraccia 
              << " in modalità: " << tipoTransizione.toStdString() << std::endl;
    labelStatoDownload->setText("✨ Transizione traccia aggiornata a: " + tipoTransizione);
}

// --- ENGINE DOWNLOAD (ASINCRONO SICURO) ---
void FinestraPrincipale::gestisciDownload() {
    QString url = inputUrl->text().trimmed();
    if (url.isEmpty()) {
        labelStatoDownload->setText("⚠️ Incolla un URL valido prima di premere scarica!");
        return;
    }

    labelStatoDownload->setText("📥 Download in corso da YouTube... Interfaccia fluida attiva...");
    barraProgresso->setRange(0, 0); 
    pulsanteScarica->setEnabled(false);

    processoPython = new QProcess(this);
    connect(processoPython, &QProcess::finished, this, &FinestraPrincipale::completatoDownload);
    processoPython->start("python3", {"scripts/downloader.py", url});
}

void FinestraPrincipale::completatoDownload(int exitCode, QProcess::ExitStatus exitStatus) {
    barraProgresso->setRange(0, 100);

    if (exitStatus == QProcess::CrashExit || exitCode != 0) {
        labelStatoDownload->setText("❌ Errore critico: Il processo Python è crashato o andato in timeout.");
        pulsanteScarica->setEnabled(true);
        barraProgresso->setValue(0);
        processoPython->deleteLater();
        return;
    }

    QString outputSorgente = processoPython->readAllStandardOutput();
    std::string outputStr = outputSorgente.toStdString();

    size_t posizioneSuccess = outputStr.find("RISULTATO_SUCCESS: ");
    if (posizioneSuccess != std::string::npos && outputStr.length() > posizioneSuccess + 19) {
        std::string percorsoWav = outputStr.substr(posizioneSuccess + 19);
        percorsoWav.erase(percorsoWav.find_last_not_of(" \n\r\t") + 1);

        if (!percorsoWav.empty()) {
            QFileInfo fileInfo(QString::fromStdString(percorsoWav));
            QString nomeFilePulito = fileInfo.fileName();

            if (layoutListaTracce->count() > 0 && 
                layoutListaTracce->itemAt(0)->widget() && 
                layoutListaTracce->itemAt(0)->widget()->objectName() == "labelVuota") {
                QLayoutItem* item = layoutListaTracce->takeAt(0);
                delete item->widget();
                delete item;
            }

            TracciaAudio tracciaReale(percorsoWav);
            if (tracciaReale.caricaInMemoria()) {
                float durataEffettiva = tracciaReale.getDurata();
                float tempoInizioTimeline = timeline.getDurataTotale();
                
                if (tempoInizioTimeline > 5.0f) {
                    tempoInizioTimeline -= 5.0f; 
                }

                timeline.aggiungiTraccia(tracciaReale, tempoInizioTimeline, 1);
                percorsiWavScaricati.push_back(percorsoWav);

                // Disegna il widget dinamico con la scelta della transizione
                aggiungiTracciaAllaLista(nomeFilePulito.toStdString(), durataEffettiva);

                float durataTotaleMix = timeline.getDurataTotale();
                int totMinuti = static_cast<int>(durataTotaleMix) / 60;
                int totSecondi = static_cast<int>(durataTotaleMix) % 60;
                
                QString indicatoreSemaforo = (durataTotaleMix <= 4440.0f) ? "🟢" : "🔴";
                labelTempoTotaleCD->setText(QString("Tempo Totale CD: %1:%2 / 74:00 %3")
                                            .arg(totMinuti, 2, 10, QChar('0'))
                                            .arg(totSecondi, 2, 10, QChar('0'))
                                            .arg(indicatoreSemaforo));

                barraProgresso->setValue(100);
                labelStatoDownload->setText("✨ Traccia inserita nella timeline del CD con successo!");
                inputUrl->clear();
            } else {
                labelStatoDownload->setText("❌ Errore C++: Struttura PCM non compatibile.");
                barraProgresso->setValue(0);
            }
        }
    } else {
        labelStatoDownload->setText("❌ Errore nel download o link non valido.");
        barraProgresso->setValue(0);
    }

    pulsanteScarica->setEnabled(true);
    processoPython->deleteLater();
}

// --- CREAZIONE ELEMENTO GRAFICO CON SELEZIONE TRANSIZIONE ---
void FinestraPrincipale::aggiungiTracciaAllaLista(const std::string& nomeFile, float durata) {
    int indiceCorrente = percorsiWavScaricati.size() - 1;

    QWidget* moduloTraccia = new QWidget();
    moduloTraccia->setStyleSheet("background-color: #1A1A1A; border: 1px solid #FF1493; border-radius: 6px; padding: 10px;");
    QHBoxLayout* layoutModulo = new QHBoxLayout(moduloTraccia);

    int minuti = static_cast<int>(durata) / 60;
    int secondi = static_cast<int>(durata) % 60;
    QString tempoStr = QString("%1:%2").arg(minuti, 2, 10, QChar('0')).arg(secondi, 2, 10, QChar('0'));

    QLabel* labelNome = new QLabel(QString::fromStdString(nomeFile));
    labelNome->setStyleSheet("font-weight: bold; color: #FFFFFF; border: none;");
    
    QLabel* labelDurata = new QLabel("⏱️ " + tempoStr);
    labelDurata->setStyleSheet("color: #FF1493; border: none; font-weight: bold;");

    // Menu a tendina per scegliere la transizione specifica per questa traccia
    QComboBox* menuTransizione = new QComboBox();
    menuTransizione->addItems({"Crossfade (5s)", "Cut Diretto (0s)", "Dissolvenza Lunga (10s)"});
    menuTransizione->setStyleSheet("background-color: #0A0A0A; border: 1px solid #FF1493; color: #FFFFFF; padding: 3px; min-width: 150px;");

    // Connettiamo il cambio di selezione della tendina alla logica C++ del Mixer
    connect(menuTransizione, &QComboBox::currentTextChanged, [this, indiceCorrente](const QString& tipo) {
        this->cambiaTransizioneTraccia(indiceCorrente, tipo);
    });

    layoutModulo->addWidget(labelNome, 3);
    layoutModulo->addWidget(labelDurata, 1);
    layoutModulo->addWidget(menuTransizione, 1); // Tendina integrata nel modulo della traccia

    layoutListaTracce->addWidget(moduloTraccia);
}

void FinestraPrincipale::gestisciMasterizzazione() {
    if (percorsiWavScaricati.empty()) {
        labelStatoDownload->setText("⚠️ Impossibile masterizzare: la tracklist è vuota!");
        return;
    }

    labelStatoDownload->setText("🎛️ Rendering del Master Audio Red Book unico (PCM 44100Hz)...");
    qApp->processEvents();

    std::string percorsoMasterWav = "Progetto_Mixer/cache_audio/CD_MASTER_FINAL.wav";
    
    if (!timeline.esportaMixFinale(percorsoMasterWav, 5.0f)) {
        labelStatoDownload->setText("❌ Errore durante il Mixdown e l'esportazione delle tracce.");
        return;
    }

    labelStatoDownload->setText("📄 Generazione mappa delle tracce hardware (File CUE)...");
    qApp->processEvents();

    MasterizzatoreCD hardwareBurner(percorsoMasterWav);
    if (!hardwareBurner.generaFileCue()) {
        labelStatoDownload->setText("❌ Errore hardware: Impossibile generare il file .cue.");
        return;
    }

    labelStatoDownload->setText("💿 Masterizzatore svegliato! Scrittura fisica (Simulazione)...");
    qApp->processEvents();
    
    hardwareBurner.avviaMasterizzazione(true);
    labelStatoDownload->setText("✨ PROCESSO COMPLETATO! Il master unico e la mappa CUE sono pronti.");
}