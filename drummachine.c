#include <avr/io.h>
#include "drummachine.h"
#include "input.h"
#include "leds.h"

#include <avr/pgmspace.h>
#include "apu.h"
#include "modulation.h"
#include "envelope.h"

#include "snare.c"
#include "kick.c"

#define STATE_PROGRAM 0
#define STATE_PLAY 1

#define BUTTON_NEXT 1
#define BUTTON_PREV 0
#define BUTTON_PLAY 24

uint8_t BD_pat[16] = {0};
uint8_t SD_pat[16] = {0};
uint8_t HH_pat[16] = {1};

uint8_t prev_input[32] = {0};

uint8_t state = STATE_PROGRAM;

uint8_t current_pattern = 0;

uint8_t* patterns[3] = {BD_pat, SD_pat, HH_pat};

uint8_t current_pos = 0;

#define cnt 30

uint8_t counter;

#define C4 197
#define D4 176
#define E4 156
#define F4 148
#define G4 131
#define A4 118
#define B4 104
#define C5 98

inline uint8_t switch_to_period(uint8_t num)
{
    uint8_t p = 0;
    switch (num) {
    case 0: p = C4; break;
    case 1: p = D4; break;
    case 2: p = E4; break;
    case 3: p = F4; break;
    case 4: p = G4; break;
    case 5: p = A4; break;
    case 6: p = B4; break;
    case 7: p = C5; break;
    }
    return p;
}


void drum_task()
{
    if (input[BUTTON_PLAY] == 1 && prev_input[BUTTON_PLAY] == 0) {
	if (state == STATE_PROGRAM) {
	    state = STATE_PLAY;
	    current_pos = 0;
	    counter = cnt;
	}
	else {
	    state = STATE_PROGRAM;
	    env3.gate = 0;
	}
    }
	
    else if (state == STATE_PROGRAM) {

	if (input[BUTTON_NEXT] == 1 && prev_input[BUTTON_NEXT] == 0 && current_pattern < 2) {
	    current_pattern++;
	}
	
	else if (input[BUTTON_PREV] == 1 && prev_input[BUTTON_PREV] == 0 && current_pattern > 0)
	    current_pattern--;
	   
	for (uint8_t i = 8; i < 24; i++) {
	    if (input[i] == 1 && prev_input[i] == 0) {
		if (i < 16)
		    patterns[current_pattern][i] ^= 1;
		else
		    patterns[current_pattern][i - 16] ^= 1; 
	    }
	}
    }

    else if (state == STATE_PLAY) {
	if (counter == cnt/2)
	    env3.gate = 0;
	if (counter-- == 0) {
	    counter = cnt;
	    
	    if (BD_pat[current_pos]) {
		dmc.sample = kick_c_raw;
		dmc.sample_length = kick_c_raw_len;
		dmc.sample_enabled = 1;
		dmc.current = 0;
	    }

	    if (SD_pat[current_pos]) {
		dmc.sample = snare_c_raw;
		dmc.sample_length = snare_c_raw_len;
		dmc.sample_enabled = 1;
		dmc.current = 0;
	    }

	    if (HH_pat[current_pos]) {
		env3.gate = 1;
	    }
	    
	    if (++current_pos == 16) current_pos = 0;

	}
	
	uint8_t i = 0;
	static uint8_t sq1_last = 0;
	static uint8_t sq2_last = 0;
	
	if (input[sq1_last] == 0) env1.gate = 0;
	if (input[sq2_last] == 0) env2.gate = 0;
	
	for (; i < 8; i++) {
	    if (input[i]) { 
		env1.gate = 1;
		bperiods[0] = switch_to_period(i) << 2;
		break;
	    }
	}
	
	for (i += 1; i < 8; i++) {
	    if (input[i]) { 
		env2.gate = 1;
		bperiods[1] = switch_to_period(i) << 2;
		break;
	    }
	}
	
	bperiods[2] = 0;
	for (i = 0; i < 8; i++) {
	    if (input[8 + i]) {
		bperiods[2] = switch_to_period(i) << 2;
		break;
	    }
	}
 
    }

    for (uint8_t i = 0; i < 25; i++) {
	prev_input[i] = input[i];
    }

}

void drum_update_leds()
{
    leds_7seg_set(current_pattern + 1);
    if (state == STATE_PROGRAM) 
	leds_7seg_dot_on();
    else
	leds_7seg_dot_off();

    if (state == STATE_PROGRAM) {
	leds[0] = 0;
	leds[1] = 0;
	for (uint8_t i = 0; i < 16; i++) {
	    if (i < 8) 
		leds[1] |= (patterns[current_pattern][i] << i);
	    else
		leds[0] |= (patterns[current_pattern][i] << (i - 8));
	}
    }

    if (state == STATE_PLAY) {
	leds[0] = 0;
	leds[1] = 0;
	if (current_pos < 8) 
	    leds[1] = 1 << current_pos;
	else
	    leds[0] = 1 << (current_pos - 8);
    }

}

