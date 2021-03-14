// #include "smartimpulsator.h"
// //#include "impulsator.h"
// // #include "blusator.h"
// #include "Streaming.h"

// template<class Engine>
// void SmartImpulsator<Engine>::setValue(uint16_t new_current_value, uint16_t new_max_value) {
// 	if(new_max_value>0) {
// 		m_max_value=new_max_value;
// 	}
// 	if (m_value != new_current_value) {
// 		m_value = new_current_value;
// 		on_rotation(new_current_value);
// 	}
// }


// template<class Engine>
// void SmartImpulsator<Engine>::on_button_click() {
// 	uint16_t value = GetValue();
// 	if(value == GetMaxValue()) {
// 		value = 0;
// 	} else {
// 		uint16_t step =  GetMaxValue() / getStepCount();
// 		value += step;
// 		if(value > GetMaxValue()) {
// 			value=GetMaxValue();
// 		}
// 	}
// 	setValue(value);
// #ifdef debug
// 	Serial<<"guzik value : "<<value<<"\n";
// #endif
// }

// template<class Engine>
// void SmartImpulsator::on_button_hold() {
// 	if (GetValue()>0){
// 		setValue(0);
// 	}else {
// 		setValue(m_max_value);
// 	}
// #ifdef debug
// 	Serial<<"guzik value : "<<GetValue()<<"\n";
// #endif
// }

// template<class Engine>
// void SmartImpulsator::on_rotation(int8_t value) {

// 	int32_t new_value = value;
// 	new_value+=m_value;
// //	Serial<<"new_value: "<<new_value<<", old_value: "<<ctx->m_value<<"\n";

// 	if(new_value<0) {
// 		new_value=0;
// 		m_on_underflow();
// 	} else if(new_value > m_max_value){
// 		new_value = m_max_value;
// 		m_on_overflow();
// 	}
// 	uint16_t old_value=m_value;
// 	m_value = new_value;

// 	if(new_value != old_value) {
// 		m_on_change();
// 	}
// }

// template<class Engine>
// void SmartImpulsator::setup_button(int pin_guzik, bool guzik_analog_pin) {
// 	if (guzik_analog_pin)
// 		m_guzik.setupUsingAnalogPin(pin_guzik);
// 	else
// 		m_guzik.setupUsingDigitalPin(pin_guzik);

// 	m_guzik.setupClickHandler([this]() { this->on_button_click(); });
// 	m_guzik.setupHoldHandler([this]() {this->on_button_hold();});
// }

// template<class Engine>
// SmartImpulsator::SmartImpulsator(uint16_t max_value, uint8_t step_count, int pin_guzik, bool guzik_analog_pin) :
// 		m_max_value(max_value),
// 		m_step_count(step_count), m_value(0),
// 		m_guzik() {
// 	if(pin_guzik>0) {
// 		setup_button(pin_guzik, guzik_analog_pin);
// 	} else {
// 		m_step_count=0;
// 	}
// 	if(max_value > 65534)
// 		max_value=65534;
// }

// void SmartImpulsator::update() {
// 	m_impulsator_update();
// 	m_guzik.update();
// }

