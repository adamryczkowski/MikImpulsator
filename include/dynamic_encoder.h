#pragma once
#include <stdint.h>
#include <timer.h>
#include "function_objects.h"
#ifdef debug
#include <Streaming.h>
#endif
#include "static_encoder.h" //for tab_C and rotation_callback_t
//Library that handles the encoder through the update calls. May loose some turns - depends on the probing frequency, but otherwise should be as good as the static version.
//The benefit of using it is the ability to use all the digital pins, not only 2 and 3.


class impulsator_dynamic {
public:
    impulsator_dynamic(int pin1, int pin2):m_pin1(pin1), m_pin2(pin2), m_bylo_zero(0), m_old_reg(0),m_stan(0),
    m_phase_1_to_2_time(50), m_phase_1_to_0_time(200), m_C(0), m_I(0), m_queued_knob_turns(0) {   
        pinMode(pin1,  INPUT_PULLUP);
		pinMode(pin2, INPUT_PULLUP);
     }
    void set_rotation_callback(rotation_callback_t rotation_callback) {
        m_rotation_callback = rotation_callback;
    }
    void update() {
        knob_update();
        m_ct_idle_timout.update();
		m_ct_stan_1_to_2.update();
		if (m_queued_knob_turns >= 4 ||
				m_queued_knob_turns <= -4) {
            // #ifdef debug
            // Serial<<"Calling rotation callback!\n";
            // #endif
			m_rotation_callback(m_queued_knob_turns / 4);
			m_queued_knob_turns=0;
		}

    }
	void knob_update() {
        uint8_t reg = digitalRead(m_pin1) + digitalRead(m_pin2)*2;
        if (reg == 0) {
            if(m_bylo_zero){
                m_old_reg=0;
                return;
            }
            m_bylo_zero=1;
        } else {
            m_bylo_zero=0;
        }

        if (reg == m_old_reg) 
            return;

        if (reg != m_old_reg) {
            if (m_stan == 0) {
                m_ct_stan_1_to_2.set_timer(
                        m_phase_1_to_0_time,
                        [this](){this->po_kalibracji();}, 0);
                m_stan = 1;//zaczęto kalibrację
            }
        }
        int idx = m_old_reg * 4 + reg;
        int incrC;
        int incrI;
        if(m_stan == 1) {
            incrC = tab_C[idx];
            if (incrC != 0) {
                incrI = 4 - incrC;
            } else {
                incrI=0;
            }
            m_C += incrC;
            m_I += incrI;
            // Serial<<"old_reg: "<<m_old_reg<<", reg: "<<reg<<", C: "<<incrC<<", I: "<<incrI<<"\n";

        } else if(m_stan == 2) {
            incrC = tab_C[idx];
            m_queued_knob_turns += incrC;
        } else if (m_stan == 3) {
            if (tab_C[idx]!=0) {
                incrI = 4 - tab_C[idx];
            } else {
                incrI=0;
            }
            m_queued_knob_turns -= incrI;
        }
        m_ct_idle_timout.set_timer(
                m_phase_1_to_0_time,
                [this](){time_out();}, 0);


        m_old_reg = reg;
	}
	void po_kalibracji() {
        if (m_C > m_I) {
            m_stan = 3;
            m_queued_knob_turns -= m_I;
        }else  {
            m_queued_knob_turns += m_C;
            m_stan = 2;
        }
        #ifdef debug
        Serial<<"po kalibracji. C: " << C << ", I: " << I <<"\n";
        #endif
    }
	void time_out() {
		m_old_reg = digitalRead(m_pin1) + digitalRead(m_pin2) * 2;
		m_ct_stan_1_to_2.stop_timer();
        #ifdef debug
        Serial<<"timeout\n";
        #endif
        m_stan = 0;
        m_C = 0;
        m_I = 0;
	}
private:
	// template<int, int>
	// friend class BasicImpulsatorInitializer;
	// template<int, int>
	// friend void setup_basic_impulsator(rotation_callback_t rotation_callback);
    int m_pin1;
    int m_pin2;
	int m_bylo_zero;
	uint8_t m_old_reg;
	int m_stan;
	mikTimer m_ct_stan_1_to_2;
	mikTimer m_ct_idle_timout;
	int32_t m_phase_1_to_2_time;
	int32_t m_phase_1_to_0_time; //ten sam czas z 2 do 0
	int16_t m_C; //liczba ćwiartek obrotów zgodnie ze wskazówkami zegara
	int16_t m_I; //liczba ćwiartek obrotów przeciwnie do wskazówkówek zegara
	int16_t m_queued_knob_turns;
	rotation_callback_t m_rotation_callback;
};


