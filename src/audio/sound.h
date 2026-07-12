#ifndef AUDIO_SOUND_H
#define AUDIO_SOUND_H

namespace audio
{
void Init();
void Update();
void Shutdown();

void Play(const char* name);
void PlayMusic();

void SetMasterVolume(float volume);
float MasterVolume();

void SetMusicOvertime(bool active);
}

#endif
