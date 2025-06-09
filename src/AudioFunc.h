#ifndef AUDIOFUNC_H
#define AUDIOFUNC_H

class AudioCmd{
    public:

        bool audio_playing = false;
        bool ledState  =  false; // LED state for indicating audio playback
        void audio_setup();
        void audioCmnd(const char *input);
        void audio_loop();
        bool isAudioPlaying() { return audio_playing; }
        void setAudioPlaying(bool playing) { audio_playing = playing; }
        void toggleLedState() { ledState = !ledState; }
        bool getLedState() { return ledState; }
};

#endif // AUDIOFUNC_H