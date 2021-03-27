#pragma once

#include <map>
#include <mciapi.h>
#include <mmsystem.h>
#include <playsoundapi.h>
#pragma comment(lib, "winmm.lib")

namespace Jelly
{
    namespace Sounds
    {
        enum Type : int8
        {
            NONE = 0,

            Jump1,
            Jump2,
            Jump3,

            PlayerDie,

            MAX_COUNT
        };

        static const char * EnumStrings[] = {
            STRY(NONE),
            STRY(Jump1),
            STRY(Jump2),
            STRY(Jump3),
            STRY(PlayerDie),
            STRY(MAX_COUNT)
        };
    }

    using namespace Sounds;

    class AudioManager : private instance_holder<AudioManager*>
    {
    public:
        static AudioManager* GetInstance() { return instance; }

        std::map<Sounds::Type, const char*> filepaths;

        AudioManager()
        {
            instance = this;
            filepaths[Jump1] = "assets/Sounds/jump1.wav";
            filepaths[Jump2] = "assets/Sounds/jump2.wav";
            filepaths[Jump3] = "assets/Sounds/jump3.wav";
            filepaths[PlayerDie] = "assets/Sounds/laser6.wav";
        }
        ~AudioManager()
        {
            filepaths.clear();
            instance = nullptr;
        }

        // https://stackoverflow.com/questions/22253074/how-to-play-or-open-mp3-or-wav-sound-file-in-c-program
        static void AudioManager::PlayFile(const char* path)
        {
#if _WIN32
            //PlaySoundA(path, nullptr, SND_FILENAME | SND_ASYNC);
            sndPlaySoundA(path, SND_FILENAME | SND_ASYNC);
#endif
        }

        static void AudioManager::PlaySoundType(Sounds::Type sndType)
        {
            if (sndType == Sounds::NONE)
                return;
            PlayFile(GetInstance()->filepaths[sndType]);
#if _WIN32
            //std::string cmdstr = "play ";
            //cmdstr.append(Sounds::EnumStrings[sndType]);
            //cmdstr.append(" from 0");
            //mciSendStringA(cmdstr.c_str(), nullptr, 0, nullptr);
#endif
        }
    };
}
