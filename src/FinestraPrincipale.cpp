#include "../include/FinestraPrincipale.h"
#include "../include/TracciaAudio.h"
#include "../include/MasterizzatoreCD.h"
#include <QApplication>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QProcess>
#include <QUrl>
#include <QDir>

FinestraPrincipale::FinestraPrincipale(QWidget *parent) : QWidget(parent) {
    setWindowTitle("CUSTOM CD-DA DAW - VISUAL TIMELINE EDITION");
    resize(1100, 750);
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

    QWidget* pannelloDownload = new QWidget();
    QHBoxLayout* layoutDownload = new QHBoxLayout(pannelloDownload);
    inputUrl = new QLineEdit();
    inputUrl->setPlaceholderText("Incolla link YouTube...");
    pulsanteScarica = new QPushButton("🟣 SCARICA");
    pulsanteCaricaLocale = new QPushButton("📁 APRI WAV");
    layoutDownload->addWidget(inputUrl, 3);
    layoutDownload->addWidget(pulsanteScarica, 1);
    layoutDownload->addWidget(pulsanteCaricaLocale, 1);

    labelStatoDownload = new QLabel("Status: Timeline vuota.");
    barraProgresso = new QProgressBar();
    barraProgresso->setFixedHeight(4);
    barraProgresso->setTextVisible(false);

    timelineVisiva = new TimelineVisiva(this);
    timelineVisiva->setFixedHeight(360);

    QWidget* pannelloControlli = new QWidget();
    QHBoxLayout* layoutControlli = new QHBoxLayout(pannelloControlli);
    pulsantePlay = new QPushButton("▶️ PLAY TIMELINE");
    pulsanteStop = new QPushButton("⏹️ STOP");
    sliderTempoMix = new QSlider(Qt::Horizontal);
    labelMinutaggioCorrente = new QLabel("00:00 / 00:00");
    layoutControlli->addWidget(pulsantePlay);
    layoutControlli->addWidget(pulsanteStop);
    layoutControlli->addWidget(sliderTempoMix, 1);
    layoutControlli->addWidget(labelMinutaggioCorrente);

    QWidget* pannelloMaster = new QWidget();
    QHBoxLayout* layoutMaster = new QHBoxLayout(pannelloMaster);
    labelTempoTotaleCD = new QLabel("Tempo Totale: 00:00 / 74:00 🟢");
    pulsanteMasterizza = new QPushButton("🔴 MASTERIZZA ISO CD-DA");
    layoutMaster->addWidget(labelTempoTotaleCD);
    layoutMaster->addStretch();
    layoutMaster->addWidget(pulsanteMasterizza);

    layoutPrincipale->addWidget(pannelloDownload);
    layoutPrincipale->addWidget(labelStatoDownload);
    layoutPrincipale->addWidget(barraProgresso);
    layoutPrincipale->addWidget(timelineVisiva);
    layoutPrincipale->addWidget(pannelloControlli);
    layoutPrincipale->addWidget(pannelloMaster);

    applicaStileCyberpunk();

    connect(pulsanteScarica, &QPushButton::clicked, this, &FinestraPrincipale::gestisciDownload);
    connect(pulsanteCaricaLocale, &QPushButton::clicked, this, &FinestraPrincipale::gestisciCaricamentoLocale);
    connect(pulsantePlay, &QPushButton::clicked, this, &FinestraPrincipale::gestisciPlay);
    connect(pulsanteStop, &QPushButton::clicked, this, &FinestraPrincipale::gestisciStop);
    connect(pulsanteMasterizza, &QPushButton::clicked, this, &FinestraPrincipale::gestisciMasterizzazione);
    
    connect(sliderTempoMix, &QSlider::sliderPressed, [this]() { isUserScribbling = true; });
    connect(sliderTempoMix, &QSlider::sliderReleased, [this]() { isUserScribbling = false; });
    connect(sliderTempoMix, &QSlider::sliderMoved, [this](int pos) { lettoreAudio->setPosition(pos); });
}

void FinestraPrincipale::sincronizzaEngineDSP() {
    timelineEngine.svuotaTimeline();
    std::vector<AudioClipItem*> clips = timelineVisiva->getClips();
    for (AudioClipItem* clip : clips) {
        timelineEngine.aggiungiTracciaVisiva(
            clip->getPercorso(),
            clip->getPosizioneTimeline(),
            clip->getOffsetInizio(),
            clip->getDurataTaglio()
        );
    }
    
    float durataTimeline = timelineEngine.getDurataMassimaTimeline();
    int min = static_cast<int>(durataTimeline) / 60;
    int sec = static_cast<int>(durataTimeline) % 60;
    labelTempoTotaleCD->setText(QString("Tempo Totale: %1:%2 / 74:00 🟢").arg(min, 2, 10, QChar('0')).arg(sec, 2, 10, QChar('0')));
}

void FinestraPrincipale::gestisciDownload() {
    QString url = inputUrl->text().trimmed();
    if (url.isEmpty()) {
        labelStatoDownload->setText("❌ Inserisci un URL valido!");
        return;
    }
    labelStatoDownload->setText("🟣 Download e conversione estrattore avviati...");
}

void FinestraPrincipale::gestisciCaricamentoLocale() {
    QString file = QFileDialog::getOpenFileName(this, "Apri file WAV", "", "Audio (*.wav)");
    if (file.isEmpty()) return;

    TracciaAudio analizzatore(file.toStdString());
    if (analizzatore.caricaInMemoria()) {
        timelineVisiva->aggiungiClip(file.toStdString(), analizzatore.getDurata(), 0);
        sincronizzaEngineDSP();
        labelStatoDownload->setText("📁 Clip importata con successo.");
    } else {
        labelStatoDownload->setText("❌ File non valido o campionamento errato.");
    }
}

void FinestraPrincipale::gestisciPlay() {
    sincronizzaEngineDSP();
    
    QDir dir;
    if (!dir.exists("Progetto_Mixer/cache_audio")) {
        dir.mkpath("Progetto_Mixer/cache_audio");
    }
    
    std::string percorsoAnteprima = "Progetto_Mixer/cache_audio/LIVE_PREVIEW.wav";
    
    labelStatoDownload->setText("🎚️ Render DSP della griglia visiva in corso...");
    qApp->processEvents();

    if (timelineEngine.esportaMixFinale(percorsoAnteprima)) {
        lettoreAudio->setSource(QUrl::fromLocalFile(QString::fromStdString(percorsoAnteprima)));
        
        // Disconnetti le vecchie connessioni prima di applicare la nuova durata
        disconnect(lettoreAudio, &QMediaPlayer::durationChanged, nullptr, nullptr);
        connect(lettoreAudio, &QMediaPlayer::durationChanged, this, [this](qint64 d) {
            sliderTempoMix->setRange(0, static_cast<int>(d));
        });
        
        lettoreAudio->play();
        timerAggiornamentoPosizione->start(100);
        labelStatoDownload->setText("🔊 In riproduzione.");
    } else {
        labelStatoDownload->setText("❌ Errore durante il Mix DSP.");
    }
}

void FinestraPrincipale::gestisciStop() {
    lettoreAudio->stop();
    timerAggiornamentoPosizione->stop();
    sliderTempoMix->setValue(0);
    labelMinutaggioCorrente->setText("00:00 / 00:00");
    labelStatoDownload->setText("⏹️ Riproduzione interrotta.");
}

void FinestraPrincipale::gestisciMasterizzazione() {
    sincronizzaEngineDSP();
    std::string percorsoMasterWav = "Progetto_Mixer/cache_audio/CD_MASTER_FINAL.wav";
    if (timelineEngine.esportaMixFinale(percorsoMasterWav)) {
        MasterizzatoreCD hardwareBurner(percorsoMasterWav);
        if (hardwareBurner.generaFileCue()) {
            hardwareBurner.avviaMasterizzazione(true);
            labelStatoDownload->setText("🔴 Masterizzazione avviata su cdrecord!");
        }
    }
}

QString FinestraPrincipale::formattaTempo(int ms) {
    int s = ms / 1000;
    return QString("%1:%2").arg(s / 60, 2, 10, QChar('0')).arg(s % 60, 2, 10, QChar('0'));
}

void FinestraPrincipale::applicaStileCyberpunk() {
    setStyleSheet("QWidget { background-color: #0A0A0A; color: #00FFFF; font-family: 'Consolas', monospace; }"
                  "QPushButton { background-color: #4B0082; color: white; border: 1px solid #FF1493; border-radius: 4px; padding: 6px 12px; font-weight: bold; }"
                  "QPushButton:hover { background-color: #FF1493; color: black; }"
                  "QLineEdit { background-color: #121212; border: 1px solid #4B0082; color: #FF1493; padding: 5px; }"
                  "QProgressBar { background-color: #111; border: 1px solid #4B0082; border-radius: 2px; text-align: center; color: white; }"
                  "QProgressBar::chunk { background-color: #FF1493; }");
}