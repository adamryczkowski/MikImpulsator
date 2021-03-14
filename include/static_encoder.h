#pragma once
#include <stdint.h>
#include <timer.h>
#include "function_objects.h"
#include <Streaming.h>
//Library that handles the encoder through the hardware interrupts for the best possible quality. 
//On Arduino Uno the encoder must be connected to pins 2 and 3, 
//On Arduino Due the encoder can be connected to more pins, e.g. 3 and 10. More choices can be found on https://www.arduino.cc/en/Hacking/PinMappingSAM3X ).


//Ignoring encoder buttons - we have click library to handle that. Here we focus exclusively on the encoder.

static constexpr uint8_t tab_C[16]={
0, 1, 3, 2,
3, 0, 2, 1,
1, 2, 0, 3,
2, 3, 1, 0};


using rotation_callback_t = FunctionObject<void(int8_t value)>;

template<int pin1, int pin2>
class impulsator_static_esr {
public:
    impulsator_static_esr static get_instance() {
        static impulsator_static_esr<pin1, pin2> mythis = impulsator_static_esr();
        return mythis;
    }
    impulsator_static_esr() {
#if defined(__arm__)
        // Arduino Due Board follows
		pinMode(pin1,  INPUT_PULLUP);
		pinMode(pin2, INPUT_PULLUP);
		attachInterrupt(digitalPinToInterrupt(pin1), impulsator_static_esr<pin1, pin2>::encoder_isr,CHANGE);
		attachInterrupt(digitalPinToInterrupt(pin2),impulsator_static_esr<pin1, pin2>::encoder_isr,CHANGE);
#elif defined(__AVR__)
        // Other AVR based Boards follows, like UNO
	pinMode(2, INPUT_PULLUP);
    pinMode(3, INPUT_PULLUP);
    attachInterrupt(0,impulsator_static_esr<pin1, pin2>::encoder_isr, CHANGE);
    attachInterrupt(1,impulsator_static_esr<pin1, pin2>::encoder_isr, CHANGE);
// Maybe this will also work (and is much more informative):
//    attachInterrupt(digitalPinToInterrupt(2), encoder_isr,CHANGE);
//    attachInterrupt(digitalPinToInterrupt(3),encoder_isr,CHANGE);
#else
#error Architecture or board not supported.
#endif

        phase_1_to_2_time = 200;
        phase_1_to_0_time = 50;

        queued_knob_turns=0;
        bylo_zero = 0;
        stan = 0;
        // ct_stan_1_to_2=new mikTimer();
        // ct_idle_timout=new mikTimer();
        C=0; //liczba ćwiartek obrotów zgodnie ze wskazówkami zegara
        I=0; //liczba ćwiartek obrotów przeciwnie do wskazówkówek zegara
    }
    static void set_rotation_callback(rotation_callback_t in_callback) {
        rotation_callback = in_callback;
    }
	static void encoder_isr() {
        noInterrupts();  //stop interrupts happening before we read pin values
#if defined(__arm__)
        // Arduino Due Board follows
		uint32_t tmp = REG_PIOC_PDSR;
		volatile uint8_t reg = ((tmp & (uint32_t(1)<<28))>>28) + ((tmp & (uint32_t(1)<<29))>>28);
#elif defined(__AVR__)
        // Other AVR based Boards follows, like UNO
		volatile uint8_t reg = PIND;
		reg = ((reg & 0b1100) >> 2); //reg = dwa bity. W tym miejscu odczytujemy D2 i D3 (tj. 3-ci i 4-ty bit PIND)
#else
#error Architecture or board not supported.
#endif
        if (reg == 0) {
            if(bylo_zero){
                interrupts();
                return;
            }
            bylo_zero=1;
        } else {
            bylo_zero=0;
        }

        if (reg != old_reg) {
            if (stan == 0) {
                ct_stan_1_to_2.set_timer(
                        phase_1_to_0_time,
                        po_kalibracji, 0);
                stan = 1;//zaczęto kalibrację
            }
        }
        int idx = old_reg * 4 + reg;
        int incrC;
        int incrI;
        if(stan == 1) {
            incrC = tab_C[idx];
            if (incrC != 0) {
                incrI = 4 - incrC;
            } else {
                incrI=0;
            }
            C += incrC;
            I += incrI;

        } else if(stan == 2) {
            incrC = tab_C[idx];
            queued_knob_turns += incrC;
        } else if (stan == 3) {
            if (tab_C[idx]!=0) {
                incrI = 4 - tab_C[idx];
            } else {
                incrI=0;
            }
            queued_knob_turns -= incrI;
        }
        ct_idle_timout.set_timer(
                phase_1_to_0_time,
                time_out, 0);


        old_reg = reg;
        interrupts(); //restart interrupts    
	}
	static void po_kalibracji() {
        if (C > I) {
            stan = 3;
            queued_knob_turns -= I;
        }else  {
            queued_knob_turns += C;
            stan = 2;
        }
        #ifdef debug
        Serial<<"po kalibracji. C: " << C << ", I: " << I <<"\n";
        #endif
    }
	static void time_out() {
	#if defined(__arm__)
	// Arduino Due Board follows
		old_reg = ((REG_PIOC_PDSR & (uint32_t(1)<<28))>>28) + ((REG_PIOC_PDSR & (uint32_t(1)<<29))>>28);
	#elif defined(__AVR__)
	// Other AVR based Boards follows, like UNO
		old_reg = ((PIND & 0b1100) >> 2);
	#else
	#error Architecture or board not supported.
	#endif
		ct_stan_1_to_2.stop_timer();
        #ifdef debug
        Serial<<"timeout\n";
        #endif
        stan = 0;
        C = 0;
        I = 0;
	}
	void update(){
		impulsator_static_esr::ct_idle_timout.update();
		impulsator_static_esr::ct_stan_1_to_2.update();
		if (impulsator_static_esr::queued_knob_turns >= 4 ||
				impulsator_static_esr::queued_knob_turns <= -4) {
            // #ifdef debug
            // Serial<<"Calling rotation callback!\n";
            // #endif
			rotation_callback(impulsator_static_esr::queued_knob_turns / 4);
			impulsator_static_esr::queued_knob_turns=0;
		}
    }
private:
	// template<int, int>
	// friend class BasicImpulsatorInitializer;
	// template<int, int>
	// friend void setup_basic_impulsator(rotation_callback_t rotation_callback);
	static int bylo_zero;
	static uint8_t old_reg;
	static int stan;
	static mikTimer ct_stan_1_to_2;
	static mikTimer ct_idle_timout;
	static int32_t phase_1_to_2_time;
	static int32_t phase_1_to_0_time; //ten sam czas z 2 do 0
	static int16_t C; //liczba ćwiartek obrotów zgodnie ze wskazówkami zegara
	static int16_t I; //liczba ćwiartek obrotów przeciwnie do wskazówkówek zegara
	static int16_t queued_knob_turns;
	static rotation_callback_t rotation_callback;
};


template<int pin1, int pin2>
rotation_callback_t impulsator_static_esr<pin1, pin2>::rotation_callback;

template<int pin1, int pin2>
int32_t impulsator_static_esr<pin1, pin2>::phase_1_to_2_time = 200;

template<int pin1, int pin2>
uint8_t impulsator_static_esr<pin1, pin2>::old_reg = 0;

template<int pin1, int pin2>
int32_t impulsator_static_esr<pin1, pin2>::phase_1_to_0_time = 50;

template<int pin1, int pin2>
int16_t impulsator_static_esr<pin1, pin2>::queued_knob_turns=0;

template<int pin1, int pin2>
int impulsator_static_esr<pin1, pin2>::bylo_zero = 0;

template<int pin1, int pin2>
int impulsator_static_esr<pin1, pin2>::stan = 0;

template<int pin1, int pin2>
mikTimer impulsator_static_esr<pin1, pin2>::ct_stan_1_to_2;

template<int pin1, int pin2>
mikTimer impulsator_static_esr<pin1, pin2>::ct_idle_timout;

template<int pin1, int pin2>
int16_t impulsator_static_esr<pin1, pin2>::C=0; //liczba ćwiartek obrotów zgodnie ze wskazówkami zegara

template<int pin1, int pin2>
int16_t impulsator_static_esr<pin1, pin2>::I=0; //liczba ćwiartek obrotów przeciwnie do wskazówkówek zegara



