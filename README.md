# StudySense

By: Thomas Burke

## Overview

StudySense is an embedded TinyML system that evaluates whether a physical room is suitable for studying or focused work by analyzing real-time environmental conditions such as sound level and room brightness.

The system runs entirely on an Arduino Nano 33 BLE Sense and classifies the environment into one of three states:

* Focus (good study environment)
* Noisy (too much environmental noise)
* Dark (insufficient lighting)

The goal is to help students and professionals evaluate their work environment and maximize productivity.

---

## Problem Statement

Modern study and work environments are often inconsistent and difficult to evaluate objectively. Factors such as background noise and lighting conditions significantly impact concentration and productivity, yet individuals typically rely on subjective judgment when selecting a workspace.

Despite many locations such as libraries and coffee shops implementing policies to reduce distractions, environmental conditions frequently vary. This project addresses the need for a real-time embedded system that can classify room conditions using onboard sensors and machine learning.

---

## Motivation

The motivation behind this project was based on my own personal struggle of finding productive study environments on campus.

As students increasingly work in flexible environments such as dorm rooms, libraries, and shared study spaces, there is a growing need for tools that help optimize focus without requiring constant user attention.

By utilizing low-power sensors and running a machine learning model directly on a microcontroller, this project demonstrates how intelligent systems can be:

* **Accessible** – low-cost hardware with no cloud dependency
* **Real-Time** – immediate environmental feedback
* **Private** – no audio or environmental data leaves the device
* **Efficient** – runs entirely on edge hardware with minimal power consumption

This project also serves as an exploration of TinyML, sensor fusion, and embedded AI deployment.

---

# Hardware

## Microcontroller

### Arduino Nano 33 BLE Sense Rev2

The Arduino Nano 33 BLE Sense Rev2 is a compact AI-enabled microcontroller board featuring multiple onboard sensors and support for TinyML applications.

---

## Sensors

### Microphone (MP34DT06JTR)

The onboard PDM microphone captures environmental sound.

Raw audio samples are converted into a Root Mean Square (RMS) value representing overall sound intensity. This measurement is used to distinguish between quiet and noisy environments.

### Ambient Light Sensor (APDS9960)

The APDS9960 sensor measures ambient light intensity.

The sensor provides:

* Red (R)
* Green (G)
* Blue (B)
* Clear (C)

channels.

The project uses the **Clear (C) channel** because it represents total unfiltered light intensity and provides an accurate measure of room brightness.

---

## Output

### RGB LED

The onboard RGB LED displays the predicted room state:

| LED State | Meaning                        |
| --------- | ------------------------------ |
| Green     | Focus / Good Study Environment |
| Red       | Noisy Environment              |
| Off       | Dark Environment               |

---

# Software Requirements

The following software was used to develop, train, and deploy the StudySense system:

* Python
* Jupyter Notebook
* TensorFlow
* TensorFlow Lite
* Scikit-Learn
* Pandas
* NumPy
* Matplotlib
* PlatformIO
* Git

---

# Setup Instructions

1. Clone the repository:

```bash
git clone https://github.com/teburke111/StudyRoomMLProject.git
cd StudyRoomMLProject
```

2. Create a Python environment:

```bash
conda create -n studysense python=3.11
conda activate studysense
```

3. Install Python dependencies:

```bash
pip install tensorflow scikit-learn pandas numpy matplotlib jupyter
```

4. Open the training notebooks:

5. Install PlatformIO in Visual Studio Code.

6. Open the `firmware` folder as a PlatformIO project.

---

# How to Collect Data

1. Upload the data collection firmware to the Arduino.

```bash
pio run -e data -t upload
```

2. Open the serial monitor:

```bash
pio device monitor
```

3. Run the Python data collection notebook.

4. Select the desired room label:

* focus
* noisy
* dark

5. Collect samples for approximately 60 seconds.

6. Save the generated CSV file.

7. Repeat the process for each room condition until a sufficient dataset has been collected.

---

# How to Train the Model

1. Open the training notebook.

2. Merge the datasets and preprocess the data.

3. Apply feature scaling using StandardScaler.

```python
scaler = StandardScaler()

X_train = scaler.fit_transform(X_train)
X_test = scaler.transform(X_test)
```

4. Train the neural network:

```python
history = model.fit(
    X_train,
    y_train,
    epochs=50,
    batch_size=16,
    validation_split=0.2
)
```

5. Evaluate the model using the test dataset.

6. Convert the trained model to TensorFlow Lite format.

7. Convert the `.tflite` file into a C header file and place it in:

```text
firmware/include/room_classifier.h
```

---

# How to Upload Firmware

1. Open the firmware project in PlatformIO.

2. Ensure the generated TensorFlow Lite header file is located in:

```text
firmware/include/room_classifier.h
```

3. Build the inference firmware:

```bash
pio run -e inference
```

4. Upload the firmware to the Arduino:

```bash
pio run -e inference -t upload
```

5. Open the serial monitor:

```bash
pio device monitor
```

6. Verify that the message below appears:

```text
Inference ready
```

---

# How to Run the Final Demo

1. Power the Arduino Nano 33 BLE Sense.

2. Place the device in the room to be evaluated.

3. Allow the microphone and light sensor to collect environmental data.

4. The onboard TensorFlow Lite model will classify the room as:

* Dark
* Focus
* Noisy

5. Observe the RGB LED output:

| LED Color | Room State |
| --------- | ---------- |
| Green     | Focus      |
| Red       | Noisy      |
| Off       | Dark       |

The system performs all inference directly on the Arduino without requiring an internet connection or external computer.

---

# YouTube Demonstration Video

A demonstration of the complete system can be viewed here:

```text
https://youtu.be/_VrTd0i7PVU
```


