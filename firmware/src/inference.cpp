#include <Arduino.h>
#include <PDM.h>
#include <Arduino_APDS9960.h>

#include "room_classifier.h"

#include <TensorFlowLite.h>
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
// #include "tensorflow/lite/version.h"

// -------------------- MODEL --------------------
const tflite::Model* model;
tflite::MicroInterpreter* interpreter;
TfLiteTensor* input;
TfLiteTensor* output;

// Tensor arena (adjust if needed)
constexpr int kTensorArenaSize = 20 * 1024;
uint8_t tensor_arena[kTensorArenaSize];

// -------------------- SCALER --------------------
const float SOUND_MEAN = 47.89593862f;
const float SOUND_STD  = 60.26405792f;

const float LIGHT_MEAN = 153.47755102f;
const float LIGHT_STD  = 133.07877254f;

// -------------------- AUDIO --------------------
short sampleBuffer[256];
volatile int samplesRead = 0;

float rmsValue = 0;

// -------------------- CALLBACK --------------------
void onPDMdata()
{
    int bytesAvailable = PDM.available();
    PDM.read(sampleBuffer, bytesAvailable);
    samplesRead = bytesAvailable / 2;
}

// -------------------- LED --------------------
void setDark()
{
    digitalWrite(LEDR, HIGH);
    digitalWrite(LEDG, HIGH);
    digitalWrite(LEDB, HIGH);
}

void setFocus()
{
    digitalWrite(LEDR, HIGH);
    digitalWrite(LEDG, LOW);
    digitalWrite(LEDB, HIGH);
}

void setNoisy()
{
    digitalWrite(LEDR, LOW);
    digitalWrite(LEDG, HIGH);
    digitalWrite(LEDB, HIGH);
}

// -------------------- SETUP --------------------
void setup()
{
    Serial.begin(115200);

    pinMode(LEDR, OUTPUT);
    pinMode(LEDG, OUTPUT);
    pinMode(LEDB, OUTPUT);

    setDark();

    // APDS9960
    if (!APDS.begin())
    {
        Serial.println("APDS failed");
        while (1);
    }

    // Microphone
    PDM.onReceive(onPDMdata);

    if (!PDM.begin(1, 16000))
    {
        Serial.println("PDM failed");
        while (1);
    }

    // Load model
    model = tflite::GetModel(room_classifier_tflite);

    static tflite::AllOpsResolver resolver;

    static tflite::MicroInterpreter static_interpreter(
        model,
        resolver,
        tensor_arena,
        kTensorArenaSize
    );

    interpreter = &static_interpreter;

    if (interpreter->AllocateTensors() != kTfLiteOk)
    {
        Serial.println("AllocateTensors failed");
        while (1);
    }

    input = interpreter->input(0);
    output = interpreter->output(0);

    Serial.println("Inference ready");
}

// -------------------- LOOP --------------------
void loop()
{
    static long sumSquares = 0;
    static int sampleCount = 0;

    // -------- AUDIO RMS --------
    if (samplesRead)
    {
        for (int i = 0; i < samplesRead; i++)
        {
            int s = sampleBuffer[i];
            sumSquares += (long)s * s;
            sampleCount++;
        }

        samplesRead = 0;
    }

    if (sampleCount > 0)
    {
        rmsValue = sqrt((float)sumSquares / sampleCount);
    }

    sumSquares = 0;
    sampleCount = 0;

    // -------- LIGHT --------
    int r, g, b, c = 0;

    if (APDS.colorAvailable())
    {
        APDS.readColor(r, g, b, c);
    }

    // -------- NORMALIZE --------
    float soundScaled = (rmsValue - SOUND_MEAN) / SOUND_STD;
    float lightScaled = (c - LIGHT_MEAN) / LIGHT_STD;

    // -------- MODEL INPUT --------
    input->data.f[0] = soundScaled;
    input->data.f[1] = lightScaled;

    // -------- INFERENCE --------
    if (interpreter->Invoke() != kTfLiteOk)
    {
        Serial.println("Invoke failed");
        return;
    }

    // -------- OUTPUT --------
    float darkProb   = output->data.f[0];
    float focusProb  = output->data.f[1];
    float noisyProb  = output->data.f[2];

    int state = 0;

    if (focusProb > darkProb && focusProb > noisyProb)
        state = 1;
    else if (noisyProb > darkProb && noisyProb > focusProb)
        state = 2;
    else
        state = 0;

    // -------- LED CONTROL --------
    if (state == 0) setDark();
    if (state == 1) setFocus();
    if (state == 2) setNoisy();

    // -------- DEBUG --------
    Serial.print("Sound:");
    Serial.print(rmsValue);
    Serial.print(" Light:");
    Serial.print(c);
    Serial.print(" State:");
    Serial.println(state);

    delay(200);
}
