#include "../include/TimelineVisiva.h"
#include <QGraphicsRectItem>
#include <QBrush>
#include <QPen>

TimelineVisiva::TimelineVisiva(QWidget *parent) 
    : QGraphicsView(parent), pixelPerSecondo(20.0f), numeroTracce(4), altezzaTraccia(60.0f) {
    
    scena = new QGraphicsScene(this);
    scena->setSceneRect(0, 0, 5000, numeroTracce * altezzaTraccia + 10.0f);
    setScene(scena);
    setRenderHint(QPainter::Antialiasing);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    disegnaGrigliaBackground();
}

void TimelineVisiva::disegnaGrigliaBackground() {
    for (int i = 0; i < numeroTracce; ++i) {
        QGraphicsRectItem* bgTraccia = new QGraphicsRectItem(0, i * altezzaTraccia, 5000, altezzaTraccia);
        bgTraccia->setBrush(i % 2 == 0 ? QBrush(QColor(20, 20, 20)) : QBrush(QColor(28, 28, 28)));
        bgTraccia->setPen(QPen(QColor(43, 0, 82), 1));
        scena->addItem(bgTraccia);
    }
}

void TimelineVisiva::aggiungiClip(const std::string& percorso, float durata, int tracciaDestinazione) {
    if (tracciaDestinazione >= numeroTracce) tracciaDestinazione = 0;
    AudioClipItem* clip = new AudioClipItem(percorso, durata, tracciaDestinazione);
    clip->setPos(0, tracciaDestinazione * altezzaTraccia + 5.0f);
    scena->addItem(clip);
    clips.push_back(clip);
}

void TimelineVisiva::svuotaTimeline() {
    scena->clear();
    clips.clear();
    disegnaGrigliaBackground();
}