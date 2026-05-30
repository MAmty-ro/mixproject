#include "../include/FinestraPrincipale.h"
#include "../include/TracciaAudio.h"
#include "../include/MixerTimeline.h"
#include "../include/MasterizzatoreCD.h"
#include <QApplication>
#include <QHBoxLayout>
#include <QFileInfo>
#include <QFileDialog>
#include <QProcess>
#include <QUrl>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QComboBox>
#include <iostream>

FinestraPrincipale::FinestraPrincipale(QWidget *parent) : QWidget(parent) {
    setWindowTitle("CUSTOM CD-DA DAW & BURNER - CYBERPUNK EDITION");
    resize(1050, 720);
    isDoubleSpeed = false;
    isUserScribbling = false;

    lettoreAudio = new QMediaPlayer(this);
    uscitaAudio = new QAudioOutput(this);
    lettoreAudio->setAudioOutput(uscitaAudio);
    uscitaAudio->setVolume(0.8f);

    timerAggiornamentoPosizione = new QTimer(this);
    connect(timerAggiornamentoPosizione, &QTimer::timeout, [this]() {
        if (!isUserScribbling && lettoreAudio->playbackState() == QMediaPlayer::PlayingState) {
            sliderTempoMix->setValue(static_cast<int>(lettoreAudio->position()));
            labelMinutaggioCorrente->setText(formattaTempo(lettoreAudio->position()) + " / " + formattaTempo(lettoreAudio->duration()));
        }
    });

    QVBoxLayout* layoutPrincipale = new QVBoxLayout(this);
    layoutPrincipale->setContentsMargins(15, 15, 15, 15);
    layoutPrincipale->setSpacing(15);

    // ==========================================
    // 1. PANNELLO INPUT
    // ==========================================
    QWidget* pannelloDownload = new QWidget();
    pannelloDownload->setObjectName("pannelloDownload");
    QHBoxLayout* layoutDownload = new QHBoxLayout(pannelloDownload);
    
    inputUrl = new QLineEdit();
    inputUrl->setPlaceholderText("Incolla qui l'URL di YouTube per scaricare...");
    
    pulsanteScarica = new QPushButton("🟣 SCARICA TRACCIA");
    pulsanteCaricaLocale = new QPushButton("📁 APRI FILE LOCALE");
    pulsanteCaricaLocale->setObjectName("pulsanteCaricaLocale");

    layoutDownload->addWidget(inputUrl, 3);
    layoutDownload->addWidget(pulsanteScarica, 1);
    layoutDownload->addWidget(pulsanteCaricaLocale, 1);

    QWidget* sottoPannelloDownload = new QWidget();
    QVBoxLayout* layoutSottoDownload = new QVBoxLayout(sottoPannelloDownload);
    layoutSottoDownload->setContentsMargins(5, 0, 5, 0);
    
    labelStatoDownload = new QLabel("Status: Pronto.");
    barraProgresso = new QProgressBar();
    barraProgresso->setFixedHeight(6);
    barraProgresso->setTextVisible(false);
    barraProgresso->setRange(0, 100);
    barraProgresso->setValue(0);
    
    layoutSottoDownload->addWidget(labelStatoDownload);
    layoutSottoDownload->addWidget(barraProgresso);

    // ==========================================
    // 2. BARRA DI AVANZAMENTO TIMELINE
    // ==========================================
    QWidget* pannelloTimelinePlayer = new QWidget();
    QVBoxLayout* layoutTimelinePlayer = new QVBoxLayout(pannelloTimelinePlayer);
    layoutTimelinePlayer->setContentsMargins(5, 0, 5, 0);
    layoutTimelinePlayer->setSpacing(4);

    sliderTempoMix = new QSlider(Qt::Horizontal);
    sliderTempoMix->setObjectName("sliderTempoMix");
    sliderTempoMix->setRange(0, 0); 

    labelMinutaggioCorrente = new QLabel("00:00 / 00:00");
    labelMinutaggioCorrente->setStyleSheet("color: #00FFFF; font-weight: bold; font-family: monospace;");

    layoutTimelinePlayer->addWidget(sliderTempoMix);
    layoutTimelinePlayer->addWidget(labelMinutaggioCorrente);

    connect(sliderTempoMix, &QSlider::sliderPressed, [this]() { isUserScribbling = true; });
    connect(sliderTempoMix, &QSlider::sliderReleased, [this]() { isUserScribbling = false; });
    connect(sliderTempoMix, &QSlider::sliderMoved, this, &FinestraPrincipale::gestisciSpostamentoManualeBarra);

    // ==========================================
    // 3. PANNELLO TRASPORTO
    // ==========================================
    QWidget* pannelloControlliAvvanzati = new QWidget();
    pannelloControlliAvvanzati->setObjectName("pannelloControlliAvvanzati");
    QHBoxLayout* layoutControlli = new QHBoxLayout(pannelloControlliAvvanzati);

    pulsantePlay = new QPushButton("▶️ PLAY");
    pulsantePlay->setObjectName("pulsantePlay");
    pulsanteStop = new QPushButton("⏹️ STOP");
    pulsanteStop->setObjectName("pulsanteStop");
    pulsanteVelocita = new QPushButton("⏩ x1");
    pulsanteVelocita->setObjectName("pulsanteVelocita");
    
    pulsanteSkipTransizione = new QPushButton("⏭️ SALTA A -10s TRANS.");
    pulsanteSkipTransizione->setObjectName("pulsanteSkipTransizione");

    layoutControlli->addWidget(pulsantePlay);
    layoutControlli->addWidget(pulsanteStop);
    layoutControlli->addWidget(pulsanteVelocita);
    layoutControlli->addWidget(pulsanteSkipTransizione);
    layoutControlli->addStretch();

    // ==========================================
    // 4. AREA TRACKLIST (SCROLLABILE)
    // ==========================================
    areaScorrimentoTracce = new QScrollArea();
    areaScorrimentoTracce->setWidgetResizable(true);
    areaScorrimentoTracce->setObjectName("areaTracce");
    
    QWidget* contenitoreTracce = new QWidget();
    layoutListaTracce = new QVBoxLayout(contenitoreTracce);
    layoutListaTracce->setAlignment(Qt::AlignTop);
    layoutListaTracce->setSpacing(10);
    areaScorrimentoTracce->setWidget(contenitoreTracce);

    QLabel* labelVuota = new QLabel("🎵 Nessuna traccia nel CD.");
    labelVuota->setObjectName("labelVuota");
    labelVuota->setAlignment(Qt::AlignCenter);
    layoutListaTracce->addWidget(labelVuota);

    // ==========================================
    // 5. PANNELLO MASTER CD
    // ==========================================
    QWidget* pannelloMaster = new QWidget();
    pannelloMaster->setObjectName("pannelloMaster");
    QHBoxLayout* layoutMaster = new QHBoxLayout(pannelloMaster);
    
    labelTempoTotaleCD = new QLabel("Tempo Totale CD: 00:00 / 74:00 🟢");
    pulsanteMasterizza = new QPushButton("🔴 MASTERIZZA CD FISICO");
    pulsanteMasterizza->setObjectName("pulsanteMasterizza");
    
    layoutMaster->addWidget(labelTempoTotaleCD);
    layoutMaster->addStretch();
    layoutMaster->addWidget(pulsanteMasterizza);

    layoutPrincipale->addWidget(pannelloDownload);
    layoutPrincipale->addWidget(sottoPannelloDownload);
    layoutPrincipale->addWidget(pannelloTimelinePlayer); 
    layoutPrincipale->addWidget(pannelloControlliAvvanzati); 
    layoutPrincipale->addWidget(areaScorrimentoTracce, 1);
    layoutPrincipale->addWidget(pannelloMaster);

    applicaStileCyberpunk();

    connect(pulsanteScarica, &QPushButton::clicked, [this]() { this->gestisciDownload(); });
    connect(pulsanteCaricaLocale, &QPushButton::clicked, [this]() { this->gestisciCaricamentoLocale(); });
    connect(pulsanteMasterizza, &QPushButton::clicked, [this]() { this->gestisciMasterizzazione(); });
    connect(pulsantePlay, &QPushButton::clicked, [this]() { this->gestisciPlay(); });
    connect(pulsanteStop, &QPushButton::clicked, [this]() { this->gestisciStop(); });
    connect(pulsanteVelocita, &QPushButton::clicked, [this]() { this->gestisciVelocita(); });
    connect(pulsanteSkipTransizione, &QPushButton::clicked, [this]() { this->gestisciSkipTransizione(); });
}

QString FinestraPrincipale::formattaTempo(int millisecondi) {
    int secondiTotali = millisecondi / 1000;
    int minuti = secondiTotali / 60;
    int secondi = secondiTotali % 60;
    return QString("%1:%2").arg(minuti, 2, 10, QChar('0')).arg(secondi, 2, 10, QChar('0'));
}

void FinestraPrincipale::applicaStileCyberpunk() {
    QString stile = 
        "QWidget { background-color: #0A0A0A; color: white; font-family: 'Segoe UI', Arial, sans-serif; }"
        "#pannelloDownload, #pannelloMaster, #pannelloControlliAvvanzati { background-color: #121212; border: 2px solid #4B0082; border-radius: 8px; padding: 8px; }"
        "QLineEdit { background-color: #1A1A1A; border: 1px solid #4B0082; border-radius: 4px; padding: 8px; color: #FF1493; font-size: 13px; }"
        "QPushButton { background-color: #4B0082; border: none; border-radius: 4px; padding: 10px 15px; font-weight: bold; font-size: 12px; }"
        "QPushButton:hover { background-color: #6A0DAD; }"
        "#pulsanteCaricaLocale { background-color: #20B2AA; color: #000000; }"
        "#pulsanteCaricaLocale:hover { background-color: #008B8B; color: #FFFFFF; }"
        "#pulsantePlay { background-color: #00FF00; color: #000000; }"
        "#pulsantePlay:hover { background-color: #22CC22; }"
        "#pulsanteStop { background-color: #FF4500; color: white; }"
        "#pulsanteStop:hover { background-color: #CD3700; }"
        "#pulsanteVelocita { background-color: #FFD700; color: black; min-width: 50px; }"
        "#pulsanteSkipTransizione { background-color: #00FFFF; color: black; }"
        "#pulsanteSkipTransizione:hover { background-color: #00CCCC; }"
        "QComboBox { background-color: #1A1A1A; border: 1px solid #00FFFF; border-radius: 4px; padding: 5px; color: #00FFFF; font-size: 11px; min-width: 170px; padding-left: 5px; }"
        "QProgressBar::chunk { background-color: #FF1493; border-radius: 3px; }"
        "QProgressBar { background-color: #222222; border: none; border-radius: 3px; }"
        "#areaTracce { background-color: #0F0F0F; border: 1px dashed #4B0082; border-radius: 6px; }"
        "QSlider::groove:horizontal { border: 1px solid #4B0082; height: 6px; background: #222; border-radius: 3px; }"
        "QSlider::sub-page:horizontal { background: #FF1493; border-radius: 3px; }"
        "QSlider::handle:horizontal { background: #00FFFF; border: 1px solid #00FFFF; width: 14px; height: 14px; margin: -4px 0; border-radius: 7px; }"
        "#pulsanteMasterizza { background-color: #FF0000; color: white; font-size: 13px; padding: 12px 25px; border-radius: 6px; }";
    setStyleSheet(stile);
}

void FinestraPrincipale::aggiungiTracciaAllaLista(int indice, const std::string& nomeFile, float durata, int tipoTransizioneAttuale) {
    QWidget* moduloTraccia = new QWidget();
    moduloTraccia->setStyleSheet("background-color: #1A1A1A; border: 1px solid #FF1493; border-radius: 6px; padding: 10px;");
    QHBoxLayout* layoutModulo = new QHBoxLayout(moduloTraccia);

    int minutes = static_cast<int>(durata) / 60;
    int secondi = static_cast<int>(durata) % 60;
    QString tempoStr = QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(secondi, 2, 10, QChar('0'));

    QLabel* labelIndex = new QLabel(QString("#%1").arg(indice + 1));
    labelIndex->setStyleSheet("color: #FF1493; font-weight: bold; border: none; font-size: 14px;");

    QLabel* labelNome = new QLabel(QString::fromStdString(nomeFile));
    labelNome->setStyleSheet("font-weight: bold; color: #FFFFFF; border: none;");
    
    QLabel* labelDurata = new QLabel("⏱️ " + tempoStr);
    labelDurata->setStyleSheet("color: #FF1493; border: none; font-weight: bold;");

    QPushButton* pulsanteElimina = new QPushButton("❌");
    pulsanteElimina->setFixedWidth(40);
    pulsanteElimina->setStyleSheet("QPushButton { background-color: #330000; color: #FF0000; border: 1px solid #FF0000; border-radius: 4px; padding: 5px; } QPushButton:hover { background-color: #FF0000; color: #FFFFFF; }");
    connect(pulsanteElimina, &QPushButton::clicked, [this, indice]() { this->rimuoviTraccia(indice); });

    layoutModulo->addWidget(labelIndex, 0);
    layoutModulo->addWidget(labelNome, 3);
    layoutModulo->addWidget(labelDurata, 1);

    // Gestione differenziata prima traccia / successive
    if (indice == 0) {
        QLabel* labelInizioCD = new QLabel("🏁 INIZIO CD (Fisso)");
        labelInizioCD->setStyleSheet("color: #888888; font-weight: bold; font-style: italic; border: none; padding-right: 20px;");
        layoutModulo->addWidget(labelInizioCD, 2);
    } else {
        QComboBox* selettoreFX = new QComboBox();
        selettoreFX->addItem("💗 Crossfade (5s)");
        selettoreFX->addItem("📉 Esponenziale");
        selettoreFX->addItem("📐 Taglio Netto");
        selettoreFX->addItem("🎚️ Linear Fade");

        selettoreFX->blockSignals(true);
        selettoreFX->setCurrentIndex(tipoTransizioneAttuale);
        selettoreFX->blockSignals(false);

        connect(selettoreFX, &QComboBox::currentIndexChanged, [this, indice](int nuovoIndex) {
            if (indice < static_cast<int>(this->tipiTransizioni.size())) {
                this->tipiTransizioni[indice] = nuovoIndex;
                this->timeline.svuotaTimeline();
                for (size_t i = 0; i < this->percorsiWavScaricati.size(); ++i) {
                    TracciaAudio t(this->percorsiWavScaricati[i]);
                    if (t.caricaInMemoria()) {
                        float tempoInizio = this->timeline.getDurataTotale();
                        int trans = this->tipiTransizioni[i];
                        if (i > 0 && tempoInizio > 5.0f && (trans == 0 || trans == 1)) {
                            tempoInizio -= 5.0f;
                        }
                        this->timeline.aggiungiTraccia(t, tempoInizio, 1);
                    }
                }
                float durataTotaleMix = this->timeline.getDurataTotale();
                int totMinuti = static_cast<int>(durataTotaleMix) / 60;
                int totSecondi = static_cast<int>(durataTotaleMix) % 60;
                QString indicatoreSemaforo = (durataTotaleMix <= 4440.0f) ? "🟢" : "🔴";
                this->labelTempoTotaleCD->setText(QString("Tempo Totale CD: %1:%2 / 74:00 %3")
                                            .arg(totMinuti, 2, 10, QChar('0'))
                                            .arg(totSecondi, 2, 10, QChar('0'))
                                            .arg(indicatoreSemaforo));
            }
        });
        layoutModulo->addWidget(selettoreFX, 2);
    }

    layoutModulo->addWidget(pulsanteElimina, 0);
    layoutListaTracce->addWidget(moduloTraccia);
}

void FinestraPrincipale::aggiornaInterfacciaTracce() {
    QLayoutItem* voceLayout;
    while ((voceLayout = layoutListaTracce->takeAt(0)) != nullptr) {
        if (voceLayout->widget()) delete voceLayout->widget();
        delete voceLayout;
    }

    timeline.svuotaTimeline();

    if (percorsiWavScaricati.empty()) {
        QLabel* labelVuota = new QLabel("🎵 Nessuna traccia nel CD. Scarica da un link o carica un file .wav dal PC.");
        labelVuota->setObjectName("labelVuota");
        labelVuota->setAlignment(Qt::AlignCenter);
        layoutListaTracce->addWidget(labelVuota);
        labelTempoTotaleCD->setText("Tempo Totale CD: 00:00 / 74:00 🟢");
        sliderTempoMix->setRange(0, 0);
        labelMinutaggioCorrente->setText("00:00 / 00:00");
        return;
    }

    for (size_t i = 0; i < percorsiWavScaricati.size(); ++i) {
        std::string percorso = percorsiWavScaricati[i];
        int transizioneDaApplicare = tipiTransizioni[i];
        
        TracciaAudio traccia(percorso);
        if (traccia.caricaInMemoria()) {
            float tempoInizio = timeline.getDurataTotale();
            if (i > 0 && tempoInizio > 5.0f) {
                if (transizioneDaApplicare == 0 || transizioneDaApplicare == 1) {
                    tempoInizio -= 5.0f; 
                }
            }
            timeline.aggiungiTraccia(traccia, tempoInizio, 1);
            aggiungiTracciaAllaLista(static_cast<int>(i), traccia.getNomeFile(), traccia.getDurata(), transizioneDaApplicare);
        }
    }

    float durataTotaleMix = timeline.getDurataTotale();
    int totMinuti = static_cast<int>(durataTotaleMix) / 60;
    int totSecondi = static_cast<int>(durataTotaleMix) % 60;
    QString indicatoreSemaforo = (durataTotaleMix <= 4440.0f) ? "🟢" : "🔴";
    labelTempoTotaleCD->setText(QString("Tempo Totale CD: %1:%2 / 74:00 %3")
                                .arg(totMinuti, 2, 10, QChar('0'))
                                .arg(totSecondi, 2, 10, QChar('0'))
                                .arg(indicatoreSemaforo));
}

void FinestraPrincipale::gestisciCaricamentoLocale() {
    QString fileSelezionato = QFileDialog::getOpenFileName(this, "Seleziona traccia audio WAV", QDir::homePath(), "Audio Files (*.wav)");
    if (fileSelezionato.isEmpty()) return;

    if (layoutListaTracce->count() > 0 && layoutListaTracce->itemAt(0)->widget() && layoutListaTracce->itemAt(0)->widget()->objectName() == "labelVuota") {
        QLayoutItem* item = layoutListaTracce->takeAt(0);
        delete item->widget();
        delete item;
    }

    percorsiWavScaricati.push_back(fileSelezionato.toStdString());
    tipiTransizioni.push_back(0); 
    aggiornaInterfacciaTracce();
}

void FinestraPrincipale::gestisciDownload() {
    QString url = inputUrl->text().trimmed();
    if (url.isEmpty()) return;

    labelStatoDownload->setText("📥 Download in corso da YouTube...");
    barraProgresso->setRange(0, 0); 
    pulsanteScarica->setEnabled(false);
    qApp->processEvents();

    QProcess* processo = new QProcess(this);
    connect(processo, &QProcess::finished, this, [this, processo](int exitCode, QProcess::ExitStatus exitStatus) {
        barraProgresso->setRange(0, 100);
        pulsanteScarica->setEnabled(true);
        if (exitStatus == QProcess::CrashExit || exitCode != 0) {
            labelStatoDownload->setText("❌ Errore critico nel download Python.");
            processo->deleteLater();
            return;
        }

        QString output = processo->readAllStandardOutput();
        std::string outStr = output.toStdString();
        size_t pos = outStr.find("RISULTATO_SUCCESS: ");
        if (pos != std::string::npos) {
            std::string path = outStr.substr(pos + 19);
            path.erase(path.find_last_not_of(" \n\r\t") + 1);

            if (layoutListaTracce->count() > 0 && layoutListaTracce->itemAt(0)->widget() && layoutListaTracce->itemAt(0)->widget()->objectName() == "labelVuota") {
                QLayoutItem* item = layoutListaTracce->takeAt(0);
                delete item->widget();
                delete item;
            }

            percorsiWavScaricati.push_back(path);
            tipiTransizioni.push_back(0); 
            aggiornaInterfacciaTracce();
            barraProgresso->setValue(100);
            inputUrl->clear();
        }
        processo->deleteLater();
    });
    processo->start("python3", {"scripts/downloader.py", url});
}

void FinestraPrincipale::rimuoviTraccia(int indice) {
    if (indice >= static_cast<int>(percorsiWavScaricati.size())) return;
    percorsiWavScaricati.erase(percorsiWavScaricati.begin() + indice);
    tipiTransizioni.erase(tipiTransizioni.begin() + indice); 
    aggiornaInterfacciaTracce();
}

void FinestraPrincipale::gestisciPlay() { 
    if (percorsiWavScaricati.empty()) return;

    std::string percorsoAnteprima = "Progetto_Mixer/cache_audio/LIVE_PREVIEW.wav";
    labelStatoDownload->setText("🎛️ Generazione DSP del flusso audio in corso...");
    qApp->processEvents();

    if (timeline.esportaMixFinale(percorsoAnteprima)) {
        lettoreAudio->setSource(QUrl::fromLocalFile(QString::fromStdString(percorsoAnteprima)));
        
        connect(lettoreAudio, &QMediaPlayer::durationChanged, this, [this](qint64 durata) {
            sliderTempoMix->setRange(0, static_cast<int>(durata));
        });

        lettoreAudio->play();
        timerAggiornamentoPosizione->start(100); 
        labelStatoDownload->setText("🔊 In riproduzione su PipeWire Audio Server.");
    }
}

void FinestraPrincipale::gestisciStop() { 
    lettoreAudio->stop();
    timerAggiornamentoPosizione->stop();
    sliderTempoMix->setValue(0);
    labelMinutaggioCorrente->setText("00:00 / 00:00");
    labelStatoDownload->setText("⏹️ Riproduzione interrotta."); 
}

void FinestraPrincipale::gestisciSpostamentoManualeBarra(int posizione) {
    lettoreAudio->setPosition(posizione);
}

void FinestraPrincipale::gestisciSkipTransizione() {
    if (percorsiWavScaricati.size() < 2) {
        labelStatoDownload->setText("⚠️ Servono almeno 2 tracce per calcolare una transizione!");
        return;
    }

    float posizioneCorrenteSecondi = static_cast<float>(lettoreAudio->position()) / 1000.0f;
    
    float tempoAccumulato = 0.0f;
    float puntoSaltoScelto = -1.0f;
    int indiceTransizioneIncontrata = 0;

    for (size_t i = 0; i < percorsiWavScaricati.size(); ++i) {
        TracciaAudio traccia(percorsiWavScaricati[i]);
        if (traccia.caricaInMemoria()) {
            int transizioneDaApplicare = tipiTransizioni[i];
            
            if (i > 0 && tempoAccumulato > 5.0f) {
                if (transizioneDaApplicare == 0 || transizioneDaApplicare == 1) {
                    tempoAccumulato -= 5.0f; 
                }
            }

            tempoAccumulato += traccia.getDurata();

            if (tempoAccumulato - 10.5f > posizioneCorrenteSecondi) {
                puntoSaltoScelto = tempoAccumulato - 10.0f;
                indiceTransizioneIncontrata = static_cast<int>(i + 1); 
                break; 
            }
        }
    }

    if (puntoSaltoScelto < 0.0f) {
        labelStatoDownload->setText("🏁 Sei già oltre l'ultima transizione disponibile del CD!");
        return;
    }

    int millisecondiSalto = static_cast<int>(puntoSaltoScelto * 1000.0f);
    
    lettoreAudio->setPosition(millisecondiSalto);
    sliderTempoMix->setValue(millisecondiSalto);
    
    labelStatoDownload->setText(QString("⏭️ Skippato a -10s dalla transizione della Traccia #%1 (%2)")
                                .arg(indiceTransizioneIncontrata)
                                .arg(formattaTempo(millisecondiSalto)));
}

void FinestraPrincipale::gestisciVelocita() {
    isDoubleSpeed = !isDoubleSpeed;
    pulsanteVelocita->setText(isDoubleSpeed ? "⏩ x2" : "⏩ x1");
    lettoreAudio->setPlaybackRate(isDoubleSpeed ? 2.0 : 1.0);
}

void FinestraPrincipale::gestisciMasterizzazione() {
    if (percorsiWavScaricati.empty()) return;
    std::string percorsoMasterWav = "Progetto_Mixer/cache_audio/CD_MASTER_FINAL.wav";
    if (!timeline.esportaMixFinale(percorsoMasterWav)) return; 
    MasterizzatoreCD hardwareBurner(percorsoMasterWav);
    if (!hardwareBurner.generaFileCue()) return;
    hardwareBurner.avviaMasterizzazione(true);
}