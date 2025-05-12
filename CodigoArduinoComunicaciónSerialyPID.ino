#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
const int sensorPin = A0;
const int pwmPin = 9;
double setpoint = 0;
double Kp = 17.92, Ti = 333.4, Td = 0.007338;
double Ki, Kd;
const double T = 0.1;
double input, output, prevError = 0, integral = 0;
const int filterSize = 25;
double filterBuffer[filterSize];
int filterIndex = 0;
bool filterReady = false;
bool pidActive = false;
double previousAverage = 0;

const double pwmMax = 255.0;

unsigned long startTime;

void setup() {
    pinMode(pwmPin, OUTPUT);
    Serial.begin(9600);
    analogReference(INTERNAL);
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Esperando PID");

    delay(1000);  

    int raw = analogRead(sensorPin);
    input = (raw * 1.1 / 1023.0) * 100.0;
    previousAverage = input;

    startTime = millis();

    Serial.print(millis());
    Serial.print(",");
    Serial.print(setpoint);
    Serial.print(",");
    Serial.println(input);
}

void loop() {
    if (Serial.available() > 0) {
        String data = Serial.readStringUntil('\n');
        int comma1 = data.indexOf(',');
        int comma2 = data.indexOf(',', comma1 + 1);
        int comma3 = data.indexOf(',', comma2 + 1);

        String setpointStr = data.substring(0, comma1);
        String KpStr = data.substring(comma1 + 1, comma2);
        String TiStr = data.substring(comma2 + 1, comma3);
        String TdStr = data.substring(comma3 + 1);

        setpoint = setpointStr.toDouble();
        Kp = KpStr.toDouble();
        Ti = TiStr.toDouble();
        Td = TdStr.toDouble();

        Ki = Kp / Ti;
        Kd = Kp * Td;
        pidActive = true;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("PID Iniciado");
    }

    int raw = analogRead(sensorPin);
    input = (raw * 1.1 / 1023.0) * 100.0;

    bool accepted = false;
    if (abs(input - previousAverage) <= 2.0) {
        filterBuffer[filterIndex] = input;
        filterIndex = (filterIndex + 1) % filterSize;
        if (filterIndex == 0) filterReady = true;
        accepted = true;
    }

    double filteredInput;
    if (filterReady) {
        filteredInput = 0.0;
        for (int i = 0; i < filterSize; i++) {
            filteredInput += filterBuffer[i];
        }
        filteredInput /= filterSize;
        if (accepted) {
            previousAverage = filteredInput;
        }
    } else {
        filteredInput = input;
        previousAverage = input;
    }

    if (pidActive) {
        double error = setpoint - filteredInput;

        
        if (error > 25) error = 25;
        if (error < -25) error = -25;

        integral += error * T;
        double derivative = (error - prevError) / T;
        output = Kp * error + Ki * integral + Kd * derivative;

        if (output < 0) output = 0;
        output = constrain(output, 0, pwmMax);
        analogWrite(pwmPin, output);

        prevError = error;
    } else {
        analogWrite(pwmPin, 0);  
    }

    // Mostrar SP y temperatura
    lcd.setCursor(0, 1);
    lcd.print("SP:");
    lcd.print(setpoint, 1);
    lcd.print("     ");

    lcd.setCursor(8, 1);
    lcd.print("T=");
    lcd.print(filteredInput, 1);
    lcd.print("     ");

    
    Serial.print(millis());
    Serial.print(",");
    Serial.print(setpoint);
    Serial.print(",");
    Serial.println(filteredInput);

    delay(100);
}
