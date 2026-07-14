#include "audio/sound.h"

#include <string>
#include <unordered_map>
#include <vector>

#include "raylib.h"

#include "assets/pack.h"

namespace audio
{
namespace
{
    const char* SoundDir = "assets/sounds/";
    const char* MusicPath = "assets/sounds/music.ogg";
    constexpr float MusicVolume = 0.10f;
    constexpr float MusicOvertimeMultiplier = 1.2f;

    constexpr int VoicesPerSound = 8;
    constexpr int MaxConcurrentVoices = 16;
    constexpr float SfxVolume = 0.5f;

    struct SoundBank
    {
        ::Sound base{};
        std::vector<::Sound> voices;
        int next = 0;
    };

    bool g_ready = false;
    bool g_musicLoaded = false;
    bool g_musicOvertime = false;
    float g_master = 1.0f;
    ::Music g_music{};
    unsigned char* g_musicData = nullptr; // kept alive: music streams from memory
    std::unordered_map<std::string, SoundBank> g_sounds;

    int ActiveVoices()
    {
        int active = 0;
        for (auto& entry : g_sounds)
            for (const ::Sound& voice : entry.second.voices)
                if (IsSoundPlaying(voice)) active++;
        return active;
    }

    float VolumeFor(const std::string& key)
    {
        if (key == "heal") return SfxVolume * 0.5f;
        return SfxVolume;
    }

    SoundBank& AcquireBank(const std::string& key)
    {
        auto it = g_sounds.find(key);
        if (it != g_sounds.end()) return it->second;

        std::string path = std::string(SoundDir) + key + ".wav";
        if (!assets::Exists(path.c_str())) path = std::string(SoundDir) + key + ".ogg";

        SoundBank bank;
        bank.base = LoadSound(path.c_str());
        bank.voices.reserve(VoicesPerSound);
        float volume = VolumeFor(key);
        for (int i = 0; i < VoicesPerSound; i++)
        {
            ::Sound voice = LoadSoundAlias(bank.base);
            SetSoundVolume(voice, volume);
            bank.voices.push_back(voice);
        }
        return g_sounds.emplace(key, std::move(bank)).first->second;
    }

    void RegisterAll()
    {
        for (const std::string& path : assets::List(SoundDir))
        {
            const char* p = path.c_str();
            if (!IsFileExtension(p, ".wav") && !IsFileExtension(p, ".ogg")) continue;
            if (TextIsEqual(GetFileName(p), "music.ogg")) continue;
            AcquireBank(GetFileNameWithoutExt(p));
        }
    }
}

void Init()
{
    if (g_ready) return;
    InitAudioDevice();
    g_ready = IsAudioDeviceReady();
    if (g_ready)
    {
        ::SetMasterVolume(g_master);
        RegisterAll();
    }
}

void Update()
{
    if (g_musicLoaded) UpdateMusicStream(g_music);
}

void Shutdown()
{
    if (!g_ready) return;
    if (g_musicLoaded)
    {
        StopMusicStream(g_music);
        UnloadMusicStream(g_music);
        g_musicLoaded = false;
    }
    if (g_musicData != nullptr)
    {
        UnloadFileData(g_musicData);
        g_musicData = nullptr;
    }
    for (auto& entry : g_sounds)
    {
        for (::Sound& voice : entry.second.voices) UnloadSoundAlias(voice);
        UnloadSound(entry.second.base);
    }
    g_sounds.clear();
    CloseAudioDevice();
    g_ready = false;
}

void Play(const char* name)
{
    if (!g_ready || name == nullptr) return;

    SoundBank& bank = AcquireBank(name);
    if (bank.voices.empty()) return;

    if (ActiveVoices() >= MaxConcurrentVoices) return;

    for (int i = 0; i < VoicesPerSound; i++)
    {
        int idx = (bank.next + i) % VoicesPerSound;
        if (!IsSoundPlaying(bank.voices[idx]))
        {
            PlaySound(bank.voices[idx]);
            bank.next = (idx + 1) % VoicesPerSound;
            return;
        }
    }
}

void SetMasterVolume(float volume)
{
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    g_master = volume;
    if (g_ready) ::SetMasterVolume(volume);
}

float MasterVolume()
{
    return g_master;
}

void PlayMusic()
{
    if (!g_ready) return;
    if (!g_musicLoaded)
    {
        // Music streams from memory: LoadMusicStream() opens the file directly
        // and bypasses the pack callback, so feed it the packed bytes instead.
        int size = 0;
        g_musicData = LoadFileData(MusicPath, &size);
        if (g_musicData == nullptr || size == 0) return;
        g_music = LoadMusicStreamFromMemory(GetFileExtension(MusicPath), g_musicData, size);
        if (g_music.frameCount == 0)
        {
            UnloadFileData(g_musicData);
            g_musicData = nullptr;
            return;
        }
        g_music.looping = true;
        g_musicLoaded = true;
        SetMusicVolume(g_music, MusicVolume * (g_musicOvertime ? MusicOvertimeMultiplier : 1.0f));
    }
    PlayMusicStream(g_music);
}

void SetMusicOvertime(bool active)
{
    if (g_musicOvertime == active) return;
    g_musicOvertime = active;
    if (g_musicLoaded)
        SetMusicVolume(g_music, MusicVolume * (active ? MusicOvertimeMultiplier : 1.0f));
}
}
