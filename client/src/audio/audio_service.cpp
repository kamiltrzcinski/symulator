#include "audio/audio_service.hpp"
#include <QUrl>

namespace symulator::client::audio {

AudioService::AudioService(QObject* parent)
    : QObject(parent)
{
    // Konfiguracja krótkich dźwięków przez QSoundEffect (szybkie odtwarzanie)
    ebiPipSound_.setSource(QUrl("qrc:/audio/ebi_pip.wav"));
    ebiPipSound_.setVolume(1.0f);

    dgtESound_.setSource(QUrl("qrc:/audio/dgt_e.wav"));
    dgtESound_.setVolume(1.0f);

    // Konfiguracja dłuższego dźwięku (np. nagrania) przez QMediaPlayer
    dgtPPlayer_.setAudioOutput(&dgtPAudioOutput_);
    dgtPPlayer_.setSource(QUrl("qrc:/audio/dgt_p.wav"));
    dgtPAudioOutput_.setVolume(1.0f);
}

AudioService::~AudioService() = default;

void AudioService::playEbiPip() {
    ebiPipSound_.play();
}

void AudioService::playDgtE() {
    dgtESound_.play();
}

void AudioService::playDgtP() {
    dgtPPlayer_.stop(); // Zatrzymanie poprzedniego, jeśli gra
    dgtPPlayer_.play();
}

void AudioService::stopAll() {
    ebiPipSound_.stop();
    dgtESound_.stop();
    dgtPPlayer_.stop();
}

} // namespace symulator::client::audio
