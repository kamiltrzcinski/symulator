#pragma once

#include <QObject>
#include <QSoundEffect>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <memory>

namespace symulator::client::audio {

class AudioService : public QObject {
    Q_OBJECT
public:
    explicit AudioService(QObject* parent = nullptr);
    ~AudioService() override;

public slots:
    // Przykładowe sloty odtwarzające konkretne dźwięki
    void playEbiPip();
    void playDgtE();
    void playDgtP();
    
    // Zatrzymywanie
    void stopAll();

private:
    QSoundEffect ebiPipSound_;
    QSoundEffect dgtESound_;
    
    QMediaPlayer dgtPPlayer_;
    QAudioOutput dgtPAudioOutput_;
};

} // namespace symulator::client::audio
