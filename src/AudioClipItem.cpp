#include "../include/AudioClipItem.h"
#include <QGraphicsSceneMouseEvent>
#include <QCursor>
#include <QBrush>
#include <QPen>
#include <QFileInfo>

AudioClipItem::AudioClipItem(const std::string& percorso, float durata, int traccia) 
    : percorsoFile(percorso), durataOriginale(durata), tracciaIndex(traccia), 
      offsetInizio(0.0f), durataTaglio(durata), posizioneTimeline(0.0f), 
      ridimensionamentoInCorso(false) {
    
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setAcceptHoverEvents(true);
    
    setRect(0, 0, durata * 20.0f, 50.0f); 
    setBrush(QBrush(QColor(75, 0, 130, 180))); 
    setPen(QPen(QColor(255, 20, 147), 1.5)); 

    nomeFile = QFileInfo(QString::fromStdString(percorso)).fileName().toStdString();
    etichettaTesto = new QGraphicsSimpleTextItem(QString::fromStdString(nomeFile), this);
    etichettaTesto->setBrush(QBrush(Qt::white));
    etichettaTesto->setPos(10, 15);
}

void AudioClipItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        if (event->pos().x() > rect().width() - 15) {
            ridimensionamentoInCorso = true;
        } else {
            QGraphicsRectItem::mousePressEvent(event);
        }
    }
}

void AudioClipItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (ridimensionamentoInCorso) {
        float nuovaLarghezza = event->pos().x();
        if (nuovaLarghezza > 20.0f && (nuovaLarghezza / 20.0f) <= durataOriginale) { 
            setRect(0, 0, nuovaLarghezza, 50.0f);
            durataTaglio = nuovaLarghezza / 20.0f;
        }
    } else {
        QGraphicsRectItem::mouseMoveEvent(event);
        posizioneTimeline = pos().x() / 20.0f;
        if (posizioneTimeline < 0.0f) { 
            posizioneTimeline = 0.0f; 
            setPos(0, pos().y()); 
        }
        setPos(pos().x(), tracciaIndex * 60.0f + 5.0f);
    }
}

void AudioClipItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    ridimensionamentoInCorso = false;
    QGraphicsRectItem::mouseReleaseEvent(event);
}

void AudioClipItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event) {
    if (event->pos().x() > rect().width() - 15) setCursor(Qt::SizeHorCursor); 
    else setCursor(Qt::SizeAllCursor);
    QGraphicsRectItem::hoverMoveEvent(event);
}