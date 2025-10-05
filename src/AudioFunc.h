#ifdef USE_AUDIO

#ifndef AUDIOFUNC_H
#define AUDIOFUNC_H

class AudioCmd{
    public:

        bool audio_playing = false;
        void audio_setup();
        void audioCmnd(const char *input);
        void audio_loop();
        bool isAudioPlaying() { return audio_playing; }
        void setAudioPlaying(bool playing) { audio_playing = playing; }
        bool getLedState();
        
};

#endif // AUDIOFUNC_H
#endif // USE_AUDIO