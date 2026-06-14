#include <Arduino.h>
#include <Arduino_APDS9960.h>
#include <PDM.h>

short sampleBuffer[256];
volatile int samplesRead = 0;

void onPDMdata()
{
    int bytesAvailable = PDM.available();
    PDM.read(sampleBuffer, bytesAvailable);
    samplesRead = bytesAvailable / 2;
}

void setup()
{
    Serial.begin(115200);

    while (!Serial);

    if (!APDS.begin())
    {
        Serial.println("Light sensor failed");
        while (1);
    }

    PDM.onReceive(onPDMdata);

    if (!PDM.begin(1, 16000))
    {
        Serial.println("Microphone failed");
        while (1);
    }

    Serial.println("START");
}

unsigned long lastSendTime = 0;

void loop()
{
    static long sumSquares = 0;
    static int peak = 0;
    static int sampleCount = 0;

    // ---- AUDIO PROCESSING ----
    if (samplesRead)
    {
        for (int i = 0; i < samplesRead; i++)
        {
            int sample = sampleBuffer[i];

            sumSquares += (long)sample * sample;

            if (abs(sample) > peak)
                peak = abs(sample);

            sampleCount++;
        }

        samplesRead = 0;
    }

    // ---- OUTPUT EVERY 1s ----
    if (millis() - lastSendTime >= 1000)
    {
        float rms = 0;

        if (sampleCount > 0)
            rms = sqrt((float)sumSquares / sampleCount);

        static float lightAvg = 0;

        if (APDS.colorAvailable())
        {
            int r, g, b, c;
            APDS.readColor(r, g, b, c);

            lightAvg = c;        
        }

        // CLEAN OUTPUT ONLY
        Serial.print(rms);
        Serial.print(",");
        Serial.println(lightAvg);

        // reset window
        sumSquares = 0;
        peak = 0;
        sampleCount = 0;
        lastSendTime = millis();
    }
}