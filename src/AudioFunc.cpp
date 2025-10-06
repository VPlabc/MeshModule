#define USE_AUDIO
#ifdef USE_AUDIO
#include "main.h"
#include "./AudioFunc.h"

#include "Arduino.h"
#include "WiFi.h"
#include "Audio.h"
#include "ARDUINO_JSON.h"
// #define I2S_BCLK      45
// #define I2S_LRC       47
// #define I2S_DOUT      46
#define I2S_BCLK      47//38
#define I2S_LRC       45//33//39
#define I2S_DOUT      46//34//37
#define LIGHT         Y8
#define LED
Audio audio;
AudioBuffer audioBuffer;
bool LightState = 0;
// {"freeRam":120,"freeSram":6869,"chipTemp":51.71379852,"mqttState":"connected","wifiMode":"STA","meshEnable":false,"loraEnable":false,"runTime":51,"resetCounter":358,"lightStatus":"off"}

#ifdef LED
#include <Adafruit_NeoPixel.h>

Adafruit_NeoPixel AudStrip = Adafruit_NeoPixel(1, 21, NEO_GRB + NEO_KHZ800);

void AudLed_setColor(uint32_t color) {
    for (int i = 0; i < AudStrip.numPixels(); i++) {
        AudStrip.setPixelColor(i, color);
    }
    AudStrip.show();
}
#endif//LED
void AudioCmd::audio_setup() {
    audioBuffer.init();
    if(audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT)){
        Serial.println("✅    Audio pins set successfully.");
    } else {
        Serial.println("❌   Failed to set audio pins.");
    }
    audio.setVolume(3);  // Set volume level (0-21)
    
    pinMode(LIGHT, OUTPUT); // Set GPIO 15 as output for LED control
    digitalWrite(LIGHT, LOW); // Initialize LED to LOW (off)

    #ifdef LED
        Adafruit_NeoPixel(1, 21, NEO_GRB + NEO_KHZ800);
        AudStrip.begin();
        AudStrip.setBrightness(100);
        AudStrip.show();
    #endif//LED
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
bool ledState  =  false; // LED state for indicating audio playback

bool AudioCmd::getLedState(){ return ledState;}

void AudioCmd::audioCmnd(const char *input)
{
    Serial.println("> Command: " + String(input));
        JSONVar inputPro = JSON.parse(input); // Parse the input string as JSON
        if (inputPro.hasOwnProperty("executeAt") && inputPro.hasOwnProperty("data")) {
            JSONVar data = inputPro["data"];
            // Light control
            if (data.hasOwnProperty("light")) {
                bool lightOn = (bool)data["light"];
                if (lightOn) {
                    digitalWrite(LIGHT, HIGH); // Turn on the LED
                    ledState = 1;
                    Serial.println("Light On");
                } else {
                    digitalWrite(LIGHT, LOW); // Turn on the LED
                    Serial.println("Light Off");
                    ledState = 0;
                }
            }
            // Volume control
            if (data.hasOwnProperty("volume")) {
                int vol = (int)data["volume"];
                if (vol >= 0 && vol <= 21) {
                    audio.setVolume(vol);
                    Serial.printf("Set volume to %d\n", vol);
                }
                if ( vol > 0 ) {
                    audio.connecttoFS(SD, "/sound2.wav");
                }
            } 
            if(data.hasOwnProperty("speech")) {
                const char* speechText = (const char*)data["speech"];
                const char* speechVoice = "vi";
                if (data.hasOwnProperty("voice")) {
                    speechVoice = (const char*)data["voice"];
                }
                Serial.printf("Speaking: %s, Voice: %s\n", speechText, speechVoice);
                audio.connecttospeech(speechText, speechVoice);
            }
            if(data.hasOwnProperty("play")) {
                char file[128] = {0};
                strncpy(file, (const char*)data["play"], sizeof(file) - 1);
                Serial.println("Playing audio...");
                audio.connecttoFS(SD, file);
            }
            if(data.hasOwnProperty("playurl")) {
                char url[256] = {0};
                strncpy(url, (const char*)data["playurl"], sizeof(url) - 1);
                Serial.printf("Playing audio from URL: %s\n", url);
                audio.connecttohost(url);
            }
            if(data.hasOwnProperty("cmd")) {
                // audio.stopAudioTask();
                const char* cmd = (const char*)data["cmd"];

                if(strcmp(cmd, "resume") == 0) {
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
                else if(strcmp(cmd, "seek") == 0 && data.hasOwnProperty("position")) {
                    int position = data["position"];
                    Serial.printf("Seeking to position: %d seconds\n", position);
                    audio.setAudioPlayPosition(position);
                }
                else if(strcmp(cmd, "balance") == 0 && data.hasOwnProperty("balance")) {
                    int balance = data["balance"];
                    Serial.printf("Setting balance to: %d\n", balance);
                    audio.setBalance(balance);
                }
                else if(strcmp(cmd, "tone") == 0 && data.hasOwnProperty("low") && data.hasOwnProperty("mid") && data.hasOwnProperty("high")) {
                    int low = data["low"];
                    int mid = data["mid"];
                    int high = data["high"];
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
                    Serial.println("  {\"cmd\":\"resume\"} : Resume audio playback.");
                    Serial.println("  {\"cmd\":\"pause\"} : Pause audio playback.");
                    Serial.println("  {\"cmd\":\"stop\"} : Stop audio playback.");
                    Serial.println("  {\"cmd\":\"seek\":<position>} : Seek to a specific position in seconds.");
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
                    audio.setVolume(7);
                }
                else if(strcmp(cmd, "exit") == 0) {
                    Serial.println("Exiting audio control.");
                    return;
                }
                else {
                    Serial.println("Unknown command."); 
                }
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
            #ifdef LED
            ledState ? AudLed_setColor(0x00ffff) : AudLed_setColor(0x000000);    
            #endif//  LED       
        }
    } else if(!audio.isRunning())  {
        if(audio_playing) {
            audio_playing = false;
            Serial.println("Audio stopped playing.");
                #ifdef LED
                AudLed_setColor(0x000000); // Set LED color to blue
                #endif// LED
        }

        if (!ledOn && currentMillis - lastBlinkTime >= 2000) {
            #ifdef LED
            AudLed_setColor(0x00ff00);
            #endif//LED
            ledOn = true;
            lastBlinkTime = currentMillis;
        }
        if (ledOn && currentMillis - lastBlinkTime >= 100) {
            #ifdef LED
            AudLed_setColor(0x000000);
            #endif//LED
            ledOn = false;
            lastBlinkTime = currentMillis;
        }
    } else if( WiFi.status() != WL_CONNECTED) {
        if (!ledOn && currentMillis - lastBlinkTime >= 1000) {
            #ifdef LED
            AudLed_setColor(0xffff00);
            #endif//LED
            ledOn = true;
            lastBlinkTime = currentMillis;
        }
        if (ledOn && currentMillis - lastBlinkTime >= 500) {
            #ifdef LED
            AudLed_setColor(0x000000);
            #endif //LED
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
#endif // USE_AUDIO