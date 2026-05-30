#ifndef FINESTRAPRINCIPALE_H
#define FINESTRAPRINCIPALE_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QScrollArea>
#include <QProcess>
#include <QComboBox> // Aggiunto per la selezione delle transizioni
#include <vector>
#include <string>
#include "MixerTimeline.h"

class FinestraPrincipale : public QWidget {
private:
    QLineEdit* inputUrl;
    QPushButton* pulsanteScarica;
    QProgressBar* barraProgresso;
    QLabel* labelStatoDownload;
    QVBoxLayout* layoutListaTracce;
    QScrollArea* areaScorrimentoTracce;
    QLabel* labelTempoTotaleCD;
    QPushButton* pulsanteMasterizza;

    // --- NUOVI CONTROLLI DI TRASPORTO AUDIO ---
    QPushButton* pulsantePlayPause;
    QPushButton* pulsanteStop;
    QComboBox* selettoreVelocita;
    bool inRiproduzione; // Stato della riproduzione attuale

    // Strutture di stato e processo asincrono
    MixerTimeline timeline;
    std::vector<std::string> percorsiWavScaricati;
    QProcess* processoPython; 

    void applicaStileCyberpunk();

public:
    FinestraPrincipale(QWidget *parent = nullptr);
    void gestisciDownload();
    void completatoDownload(int exitCode, QProcess::ExitStatus exitStatus);
    void gestisciMasterizzazione();
    void aggiungiTracciaAllaLista(const std::string& nomeFile, float durata);
    
    // --- NUOVE FUNZIONI DI CONTROLLO LOGICO ---
    void gestisciPlayPause();
    void gestisciStop();
    void cambiaVelocita(const QString& velocita);
    void cambiaTransizioneTraccia(int indiceTraccia, const QString& tipoTransizione);
};

#endif // FINESTRAPRINCIPALE_H