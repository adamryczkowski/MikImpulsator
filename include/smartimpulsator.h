#pragma once
#include "Arduino.h"
#include "click.h"
#include "static_encoder.h"
#include "Streaming.h"

//zdarzenia emitowane:
// OnOverflow - wypalane, gdy przekręci się impulsator ponad max
// OnUnderflow - wypalane, gdy przekręci się impulsator poniżej min


template<class EncoderEngine>
class SmartImpulsator {
public:
	SmartImpulsator(EncoderEngine engine, uint16_t max_value, uint8_t step_count = 3, int pin_guzik=-1, bool guzik_analog_pin=false) :
			m_engine(engine),
			m_max_value(max_value),
			m_step_count(step_count), m_value(0),
			m_guzik() {
		if(pin_guzik>0) {
			setup_button(pin_guzik, guzik_analog_pin);
		} else {
			m_step_count=0;
		}
		if(max_value > 65534)
			max_value=65534;
		m_engine.set_rotation_callback([this](int8_t value){this->on_rotation(value);});
		#ifdef debug
		Serial<<"Impulsator set up\n";
		#endif
	}

	uint16_t GetValue() {return m_value;}
	uint16_t GetMaxValue() {return m_max_value;}
	void setValue(uint16_t new_current_value, uint16_t new_max_value=0) {
		if(new_max_value>0) {
			m_max_value=new_max_value;
		}
		if (m_value != new_current_value) {
			m_value = new_current_value;
			on_rotation(new_current_value);
		}
	}
	void setOnChange(FunctionObject<void(void)> on_change) {
		m_on_change = on_change;
	}
	void setOnUnderflow(FunctionObject<void(void)> on_underflow) {
		m_on_underflow = on_underflow;
	}
	void setOnOverflow(FunctionObject<void(void)> on_overflow) {
		m_on_overflow = on_overflow;
	}
	uint8_t getStepCount() {return m_step_count;}
	void update() {
		m_engine.update();
		m_guzik.update();
	}
	template<int pin1, int pin2>
	friend SmartImpulsator<impulsator_static_esr<pin1, pin2>>& make_smart_impulsator(uint16_t max_value, uint8_t step_count, int pin_guzik, bool guzik_analog_pin);
private:
	void on_rotation(int8_t value) {
		int32_t new_value = value;
		new_value+=m_value;
		#ifdef debug
		Serial<<"new_value: "<<new_value<<", old_value: "<<m_value<<"\n";
		#endif
		if(new_value<0) {
			new_value=0;
			m_on_underflow();
		} else if(new_value > m_max_value){
			new_value = m_max_value;
			m_on_overflow();
		}
		uint16_t old_value=m_value;
		m_value = new_value;

		if(new_value != old_value) {
			m_on_change();
		}
	}
	void setup_button(int pin_guzik, bool guzik_analog_pin) {
		if (guzik_analog_pin)
			m_guzik.setupUsingAnalogPin(pin_guzik);
		else
			m_guzik.setupUsingDigitalPin(pin_guzik);

		m_guzik.setupClickHandler([this]() { this->on_button_click(); });
		m_guzik.setupHoldHandler([this]() {this->on_button_hold();});
	}
	void on_button_click() {
		uint16_t value = GetValue();
		if(value == GetMaxValue()) {
			value = 0;
		} else {
			uint16_t step =  GetMaxValue() / getStepCount();
			value += step;
			if(value > GetMaxValue()) {
				value=GetMaxValue();
			}
		}
		setValue(value);
	#ifdef debug
		Serial<<"guzik value : "<<value<<"\n";
	#endif
	}
	void on_button_hold() {
		if (GetValue()>0){
			setValue(0);
		}else {
			setValue(m_max_value);
		}
	#ifdef debug
		Serial<<"guzik value : "<<GetValue()<<"\n";
	#endif
	}

	EncoderEngine m_engine;
	uint16_t m_max_value;
	uint8_t  m_step_count;
	uint16_t m_value;
	Guzik m_guzik;
	FunctionObject<void(void)> m_on_overflow;
	FunctionObject<void(void)> m_on_underflow;
	FunctionObject<void(void)> m_on_change;
	// FunctionObject<void(void)> m_impulsator_update;
};

template<int pin1, int pin2>
using StaticSmartImpulsator = SmartImpulsator<impulsator_static_esr<pin1, pin2>>;

template<int pin1, int pin2>
SmartImpulsator<impulsator_static_esr<pin1, pin2>>& make_smart_impulsator(uint16_t max_value, uint8_t step_count, int pin_guzik=-1, bool guzik_analog_pin=false)
 {
	static SmartImpulsator<impulsator_static_esr<pin1, pin2>> smart_impulsator = SmartImpulsator<impulsator_static_esr<pin1, pin2>>(impulsator_static_esr<pin1, pin2>(), 
			max_value, step_count, pin_guzik, guzik_analog_pin);
	return smart_impulsator;
}
