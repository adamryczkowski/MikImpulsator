// #define debug
#include <Arduino.h>
#include <smartimpulsator.h>
#include <Streaming.h>

StaticSmartImpulsator<3,10>* impulsator;

void ShowChange() {
    Serial<<"Impulsator: " << impulsator->GetValue() << ".\n";
}

void setup() {
// write your initialization code here
    Serial.begin(9600);
    pinMode(LED_BUILTIN, OUTPUT);
    impulsator = &make_smart_impulsator<3,10>(30, 3, -1, false);

    impulsator->setOnChange(ShowChange);
    Serial<<"Allo!\n";
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
// write your code here
    impulsator->update();
}