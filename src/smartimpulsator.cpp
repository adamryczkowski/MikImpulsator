// #define debug
#include <Arduino.h>
#include <smartimpulsator.h>
#ifdef debug
#include <Streaming.h>
#endif


using Impulsator1 = DynamicSmartImpulsator;
using Impulsator2 = DynamicSmartImpulsator;

Impulsator1 impulsator1 = make_smart_impulsator_dynamic(4, 5, 30, 3, -1, false);
Impulsator2 impulsator2 = make_smart_impulsator_dynamic(9, 8, 30, 3, -1, false);

void ShowChange1() {
#ifdef debug
    Serial<<"Impulsator1: " << impulsator1.GetValue() << ".\n";
#endif
}

void ShowChange2() {
#ifdef debug
    Serial<<"Impulsator2: " << impulsator2.GetValue() << ".\n";
#endif
}

void setup() {
// write your initialization code here
    Serial.begin(9600);
    pinMode(LED_BUILTIN, OUTPUT);

    impulsator1.setOnChange(ShowChange1);
    impulsator2.setOnChange(ShowChange2);
    pinMode(LED_BUILTIN, OUTPUT);
#ifdef debug
    Serial<<"Allo!\n";
#endif
}

void loop() {
// write your code here
    impulsator1.update();
    impulsator2.update();
}