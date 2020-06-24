#include "Music.h"
#include "helper.h"

Music::Music(char* trackName, uint8_t trackNameLen) {
    this->trackNameLen = trackNameLen;
    // The track name is stored as a NON Null terminated string, with 3 spaces at the end
    // This makes it easier for Audio Display to display a wrap text animation
    // The text length is still the original track name length
    this->trackName = new char[trackNameLen+3]{' '};
    memcpy(&this->trackName[0], trackName, trackNameLen);
    LOG_DEBG(F_TC("New music. Track name: %s\n"), trackName);
}

Music::~Music() {
    if (this->trackName != nullptr) {
        delete[] this->trackName;
    }
    if (this->trackArtist != nullptr) {
        delete[] this->trackArtist;
    }
}

void Music::setArtist(char* artistName, uint8_t artistNameLen) {
    this->artistNameLen = artistNameLen;
    this->trackArtist = new char[artistNameLen+1]{};
    memcpy(this->trackArtist, artistName, artistNameLen);
    LOG_DEBG(F_TC("Track artist: %s\n"), trackArtist);
}

void Music::setDuration(int secs) {
    this->duration_ms = (long)secs * 1000;
    LOG_DEBG(F_TC("Music duration is %d seconds, playing\n"), secs);
    this->playing = true;
}

void Music::setPosition(int secs) {
    this->position_ms = (long)secs * 1000;
    LOG_DEBG(F_TC("Music seeked to %d seconds\n"), secs);
}

void Music::setPlayStatus(bool playing) {
    this->playing = playing;
    if (this->playing) {
        LOG_DEBG(F_TC("Music playing\n"));
    } else {
         LOG_DEBG(F_TC("Music paused\n"));
    }
}

int Music::getDuration() {
    return (int)(this->duration_ms / 1000);
}

bool Music::isPlaying() {
    return this->playing;
}

int Music::getPosition() {
    return (int)(this->position_ms / 1000);
}

const char* Music::getArtistName() {
    return this->trackArtist;
}

const char* Music::getTrackName() {
    return this->trackName;
}

uint8_t Music::getArtistNameLen() {
    return this->artistNameLen;
}

uint8_t Music::getTrackNameLen() {
    return this->trackNameLen;
}

void Music::update() {
    if (playing && millis() - lastUpdate >= 1000) {
        this->position_ms += (millis() - lastUpdate);
        lastUpdate = millis();
    } else if (!playing) {
        lastUpdate = millis();
    }
}
