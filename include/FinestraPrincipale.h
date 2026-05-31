#ifndef FINESTRAPRINCIPALE_H
#define FINESTRAPRINCIPALE_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QSlider>
#include <QTimer>
#include <QVBoxLayout>
#include <vector>
#include <string>
#include <QMediaPlayer>
#include <QAudioOutput>

#include "TimelineVisiva.h"
#include "MixerTimeline.h"

class FinestraPrincipale : public QWidget {
    Q_OBJECT

private:
    QLineEdit* inputUrl;
    QPushButton* pulsanteScarica;
    QPushButton* pulsanteCaricaLocale;
    QLabel* labelStatoDownload;
    QProgressBar* barraProgresso;
    TimelineVisiva* timelineVisiva;
    QLabel* labelTempoTotaleCD;
    QPushButton* pulsanteMasterizza;
    QPushButton* pulsantePlay;
    QPushButton* pulsanteStop;
    QSlider* sliderTempoMix;              
    QLabel* labelMinutaggioCorrente;      

    QMediaPlayer* lettoreAudio;
    QAudioOutput* uscitaAudio;
    QTimer* timerAggiornamentoPosizione;  
    bool isUserScribbling;                 
    MixerTimeline timelineEngine; 

    void applicaStileCyberpunk();
    QString formattaTempo(int millisecondi);
    void sincronizzaEngineDSP(); 

    void gestisciDownload();
    void gestisciCaricamentoLocale();
    void gestisciPlay();
    void gestisciStop();
    void gestisciMasterizzazione();

public:
    explicit FinestraPrincipale(QWidget *parent = nullptr);
    ~FinestraPrincipale() = default;
};

#endif // FINESTRAPRINCIPALE_H