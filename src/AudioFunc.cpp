#include "./AudioFunc.h"

#include "Arduino.h"
#include "WiFi.h"
#include "Audio.h"
#include "ARDUINO_JSON.h"
#define I2S_BCLK      45
#define I2S_LRC       47
#define I2S_DOUT      46
Audio audio;
AudioBuffer audioBuffer;

#include <Adafruit_NeoPixel.h>

Adafruit_NeoPixel AudStrip = Adafruit_NeoPixel(1, 21, NEO_GRB + NEO_KHZ800);

void AudLed_setColor(uint32_t color) {
    for (int i = 0; i < AudStrip.numPixels(); i++) {
        AudStrip.setPixelColor(i, color);
    }
    AudStrip.show();
}

void AudioCmd::audio_setup() {
    if(audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT)){
        Serial.println("✅    Audio pins set successfully.");
    } else {
        Serial.println("❌   Failed to set audio pins.");
    }
    audio.setVolume(3);  // Set volume level (0-21)
 //  *** radio streams ***
    // audio.connecttohost("http://stream.antennethueringen.de/live/aac-64/stream.antennethueringen.de/"); // aac
//  audio.connecttohost("http://mcrscast.mcr.iol.pt/cidadefm");                                         // mp3
//  audio.connecttohost("http://www.wdr.de/wdrlive/media/einslive.m3u");                                // m3u
//  audio.connecttohost("https://stream.srg-ssr.ch/rsp/aacp_48.asx");                                   // asx
//  audio.connecttohost("http://tuner.classical102.com/listen.pls");                                    // pls
//  audio.connecttohost("http://stream.radioparadise.com/flac");                                        // flac
//  audio.connecttohost("http://stream.sing-sing-bis.org:8000/singsingFlac");                           // flac (ogg)
//  audio.connecttohost("http://s1.knixx.fm:5347/dein_webradio_vbr.opus");                              // opus (ogg)
//  audio.connecttohost("http://stream2.dancewave.online:8080/dance.ogg");                              // vorbis (ogg)
//  audio.connecttohost("http://26373.live.streamtheworld.com:3690/XHQQ_FMAAC/HLSTS/playlist.m3u8");    // HLS
//  audio.connecttohost("http://eldoradolive02.akamaized.net/hls/live/2043453/eldorado/master.m3u8");   // HLS (ts)
//  *** web files ***
//  audio.connecttohost("https://github.com/schreibfaul1/ESP32-audioI2S/raw/master/additional_info/Testfiles/Pink-Panther.wav");        // wav
//  audio.connecttohost("https://github.com/schreibfaul1/ESP32-audioI2S/raw/master/additional_info/Testfiles/Santiano-Wellerman.flac"); // flac
//  audio.connecttohost("https://github.com/schreibfaul1/ESP32-audioI2S/raw/master/additional_info/Testfiles/Olsen-Banden.mp3");        // mp3
//  audio.connecttohost("https://github.com/schreibfaul1/ESP32-audioI2S/raw/master/additional_info/Testfiles/Miss-Marple.m4a");         // m4a (aac)
//  audio.connecttohost("https://github.com/schreibfaul1/ESP32-audioI2S/raw/master/additional_info/Testfiles/Collide.ogg");             // vorbis
//  audio.connecttohost("https://github.com/schreibfaul1/ESP32-audioI2S/raw/master/additional_info/Testfiles/sample.opus");             // opus
//  *** local files ***
//  audio.connecttoFS(SD, "/test.wav");     // SD 
    // audio.connecttoFS(SD , "/hello1.mp3");  
}

void AudioCmd::audioCmnd(const char *input)
{
    Serial.println("> Command: " + String(input));
        JSONVar inputPro = JSON.parse(input); // Parse the input string as JSON
        if(inputPro.hasOwnProperty("data")) {
            // audio.stopAudioTask();
            audioBuffer.init();
            audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
            const char* cmd = (const char*)inputPro["data"]["cmd"];
            char file[128] = {0};
            char url[256] = {0};
            char text[256] = {0};
            char voice[32] = "en";
            int volume = 3;
            bool lightValue = (bool)inputPro["data"]["light"];

            Serial.print("Light value received: ");
            Serial.println(lightValue);
            if (lightValue) {
                Serial.println("Turning on LED.");
                digitalWrite(15, HIGH); // Turn on the LED
            } else {
                Serial.println("Turning off LED.");
                digitalWrite(15, LOW); // Turn off the LED
            }
            if(inputPro.hasOwnProperty("file")) {
            strncpy(file, (const char*)inputPro["data"]["file"], sizeof(file) - 1);
            }
            if(inputPro.hasOwnProperty("url")) {
            strncpy(url, (const char*)inputPro["data"]["url"], sizeof(url) - 1);
            }
            if(inputPro.hasOwnProperty("speech")) {
            strncpy(text, (const char*)inputPro["data"]["speech"], sizeof(text) - 1);
            }
            if(inputPro.hasOwnProperty("voice")) {
            strncpy(voice, (const char*)inputPro["data"]["voice"], sizeof(voice) - 1);
            }
            if(inputPro.hasOwnProperty("volume")) {
                volume = (int)inputPro["data"]["volume"];
                if(volume > 7) {
                    volume = 7; // Limit volume to max 21
                }
                audio.setVolume(volume); // Set volume (0-21)
            }

            if(strcmp(cmd, "play") == 0 && strlen(file) > 0) {
            Serial.println("Playing audio...");
            audio.connecttoFS(SD, file);
            }
            else if(strlen(text) > 0) {// Handle text-to-speech
                if(strlen(voice) == 0) {
                    strncpy(voice, "vi", sizeof(voice) - 1); // Default voice
                }
            Serial.printf("Playing speech: %s, Voice: %s\n", text, voice);
            audio.connecttospeech(text, voice);
            }
            else if(strcmp(cmd, "playurl") == 0 && strlen(url) > 0) {
            Serial.printf("Playing audio from URL: %s\n", url);
            audio.connecttohost(url);
            }
            else if(strcmp(cmd, "resume") == 0) {
            Serial.println("Resuming audio...");
            audio.pauseResume();
            }
            else if(strcmp(cmd, "pause") == 0) {
            Serial.println("Pausing audio...");
            audio.pauseResume();
            }
            else if(strcmp(cmd, "stop") == 0) {
            Serial.println("Stopping audio...");
            audio.stopSong();audio_playing = false;
            }
            else if(strcmp(cmd, "seek") == 0 && inputPro.hasOwnProperty("position")) {
            int position = inputPro["data"]["position"];
            Serial.printf("Seeking to position: %d seconds\n", position);
            audio.setAudioPlayPosition(position);
            }
            else if(strcmp(cmd, "volume") == 0 && inputPro.hasOwnProperty("level")) {
            int level = inputPro["data"]["level"];
            Serial.printf("Setting volume to: %d\n", level);
            audio.setVolume(level);
            }
            else if(strcmp(cmd, "balance") == 0 && inputPro.hasOwnProperty("balance")) {
            int balance = inputPro["data"]["balance"];
            Serial.printf("Setting balance to: %d\n", balance);
            audio.setBalance(balance);
            }
            else if(strcmp(cmd, "tone") == 0 && inputPro.hasOwnProperty("low") && inputPro.hasOwnProperty("mid") && inputPro.hasOwnProperty("high")) {
            int low = inputPro["data"]["low"];
            int mid = inputPro["data"]["mid"];
            int high = inputPro["data"]["high"];
            Serial.printf("Setting tone - Low: %d, Mid: %d, High: %d\n", low, mid, high);
            audio.setTone(low, mid, high);
            }
            else if(strcmp(cmd, "status") == 0) {
            Serial.printf("Audio status - Volume: %d, Position: %d/%d sec, Duration: %d sec\n",
                      audio.getVolume(),
                      audio.getAudioCurrentTime(),
                      audio.getAudioFileDuration(),
                      audio.getAudioFileDuration());
            }
            else if(strcmp(cmd, "help") == 0) {
            Serial.println("Available commands:");
            Serial.println("  {\"cmd\":\"play\",\"file\":<file>} : Play a specified audio file.");
            Serial.println("  {\"speech\":<text>, \"voice\":<voice>} : Convert text to speech.");
            Serial.println("  {\"cmd\":\"playurl\",\"url\":<url>} : Play audio from a specified URL.");
            Serial.println("  {\"cmd\":\"resume\"} : Resume audio playback.");
            Serial.println("  {\"cmd\":\"pause\"} : Pause audio playback.");
            Serial.println("  {\"cmd\":\"stop\"} : Stop audio playback.");
            Serial.println("  {\"cmd\":\"seek\":<position>} : Seek to a specific position in seconds.");
            Serial.println("  {\"cmd\":\"volume\",\"level\":<level>} : Set audio volume (0-21).");
            Serial.println("  {\"cmd\":\"balance\",\"value\":<value>} : Set audio balance (-100 to 100).");
            Serial.println("  {\"cmd\":\"tone\",\"low\":<value>, \"mid\":<value>, \"high\":<value>}} : Set audio tone.");
            Serial.println("  {\"cmd\":\"status\"} : Get current audio status.");
            Serial.println("  {\"cmd\":\"clear\"} : Clear audio buffer and reset settings.");
            Serial.println("  {\"cmd\":\"help\"} : Show this help message.");
            Serial.println("  {\"cmd\":\"exit\"} : Exit audio control.");
            }
            else if(strcmp(cmd, "clear") == 0) {
            Serial.println("Clearing audio buffer...");
            audio.stopSong();
            audio.setFileLoop(false);
            audio.forceMono(false);
            audio.setBalance(0);
            audio.setVolume(18);
            }
            else if(strcmp(cmd, "exit") == 0) {
            Serial.println("Exiting audio control.");
            return;
            }
            else {
            Serial.println("Unknown command.");
            }

        } else {
            Serial.println("Failed to parse JSON input.");
        }
}

static unsigned long lastBlinkTime = 0;
static bool ledOn = false;
unsigned long currentMillis = millis();
void AudioCmd::audio_loop()
{
    if(Serial.available()) {
        char input[256];
        size_t len = Serial.readBytesUntil('\n', input, sizeof(input) - 1);
        input[len] = '\0';
        audioCmnd(input);
    }

    if(audio.isRunning()) {
        if(!audio_playing) {
            audio_playing = true;
            Serial.println("Audio started playing.");
        }
        static uint32_t lastTime = 0;
        if(millis() - lastTime > 1000) { // every second
            lastTime = millis();
            Serial.printf("Audio is running. Volume: %d, Position: %d/%d sec, Duration: %d sec\n", 
                          audio.getVolume(), 
                          audio.getAudioCurrentTime(), 
                          audio.getAudioFileDuration(), 
                          audio.getAudioFileDuration());
            ledState = !ledState; // Toggle LED state
            ledState ? AudLed_setColor(0x00ffff) : AudLed_setColor(0x000000);             
        }
    } else if(!audio.isRunning())  {
        if(audio_playing) {
            audio_playing = false;
            Serial.println("Audio stopped playing.");
                AudLed_setColor(0x000000); // Set LED color to blue

        }

        if (!ledOn && currentMillis - lastBlinkTime >= 2000) {
            AudLed_setColor(0x00ff00);
            ledOn = true;
            lastBlinkTime = currentMillis;
        }
        if (ledOn && currentMillis - lastBlinkTime >= 100) {
            AudLed_setColor(0x000000);
            ledOn = false;
            lastBlinkTime = currentMillis;
        }
    } else if( WiFi.status() != WL_CONNECTED) {
        if (!ledOn && currentMillis - lastBlinkTime >= 1000) {
            AudLed_setColor(0xffff00);
            ledOn = true;
            lastBlinkTime = currentMillis;
        }
        if (ledOn && currentMillis - lastBlinkTime >= 500) {
            AudLed_setColor(0x000000);
            ledOn = false;
            lastBlinkTime = currentMillis;
        }
    }
    
        audio.loop();
}

//------------------EVENTS----------------------------------------------------------------------------------------------
void audio_info(const char *info){
    Serial.printf("info: %s\n", info);
}
void audio_id3data(const char *info){  //id3 metadata
    Serial.print("id3data     ");Serial.println(info);
}
void audio_eof_mp3(const char *info){  //end of file
    Serial.print("eof_mp3     ");Serial.println(info);
}
void audio_showstation(const char *info){
    Serial.print("station     ");Serial.println(info);
}
void audio_showstreamtitle(const char *info){
    Serial.print("streamtitle ");Serial.println(info);
}
void audio_bitrate(const char *info){
    Serial.print("bitrate     ");Serial.println(info);
}
void audio_commercial(const char *info){  //duration in sec
    Serial.print("commercial  ");Serial.println(info);
}
void audio_icyurl(const char *info){  //homepage
    Serial.print("icyurl      ");Serial.println(info);
}
void audio_lasthost(const char *info){  //stream URL played
    Serial.print("lasthost    ");Serial.println(info);
}
void audio_eof_speech(const char *info){
    Serial.print("eof_speech  ");Serial.println(info);
}