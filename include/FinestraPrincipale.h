#ifndef FINESTRAPRINCIPALE_H
#define FINESTRAPRINCIPALE_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QScrollArea>
#include <QSlider>
#include <QTimer>
#include <QVBoxLayout>
#include <vector>
#include <string>

#include <QMediaPlayer>
#include <QAudioOutput>

#include "MixerTimeline.h"

class FinestraPrincipale : public QWidget {
    Q_OBJECT

private:
    QLineEdit* inputUrl;
    QPushButton* pulsanteScarica;
    QPushButton* pulsanteCaricaLocale;
    QLabel* labelStatoDownload;
    QProgressBar* barraProgresso;
    QScrollArea* areaScorrimentoTracce;
    QVBoxLayout* layoutListaTracce;
    QLabel* labelTempoTotaleCD;
    QPushButton* pulsanteMasterizza;
    
    // Controlli di trasporto avanzati
    QPushButton* pulsantePlay;
    QPushButton* pulsanteStop;
    QPushButton* pulsanteVelocita;
    QPushButton* pulsanteSkipTransizione; 
    QSlider* sliderTempoMix;              
    QLabel* labelMinutaggioCorrente;      

    QMediaPlayer* lettoreAudio;
    QAudioOutput* uscitaAudio;
    QTimer* timerAggiornamentoPosizione;  
    bool isUserScribbling;                 

    MixerTimeline timeline;
    std::vector<std::string> percorsiWavScaricati;
    std::vector<int> tipiTransizioni; 
    bool isDoubleSpeed;

    void applicaStileCyberpunk();
    void aggiungiTracciaAllaLista(int indice, const std::string& nomeFile, float durata, int tipoTransizioneAttuale);
    void aggiornaInterfacciaTracce();
    QString formattaTempo(int millisecondi);

    void gestisciDownload();
    void gestisciCaricamentoLocale();
    void rimuoviTraccia(int indice);
    void gestisciPlay();
    void gestisciStop();
    void gestisciVelocita();
    void gestisciSkipTransizione();
    void gestisciSpostamentoManualeBarra(int posizione);
    void gestisciMasterizzazione();

public:
    explicit FinestraPrincipale(QWidget *parent = nullptr);
    ~FinestraPrincipale() = default;
};

#endif // FINESTRAPRINCIPALE_H