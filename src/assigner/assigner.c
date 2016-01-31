/*
  Copyright 2014-2015 Johan Fjeldtvedt 

  This file is part of NESIZER.

  NESIZER is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  NESIZER is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with NESIZER.  If not, see <http://www.gnu.org/licenses/>.



  Channel note assignment

  Assigns notes from MIDI or button input to sound channels.
*/

#include <avr/io.h>
#include <avr/pgmspace.h>
#include "modulation/modulation.h"
#include "portamento/portamento.h"
#include "apu/apu.h"
//#include "lfo/lfo.h"
#include "envelope/envelope.h"
#include "assigner/assigner.h"
#include "sample/sample.h"
#include "modulation/periods.h"
#include "midi/midi.h"

enum arp_mode assigner_arp_mode;
int8_t assigner_arp_range;
int8_t assigner_arp_channel;
int8_t assigner_arp_rate;
int8_t assigner_arp_sync;

enum assigner_mode assigner_mode = MONO;

uint8_t assigned_notes[5];

int8_t midi_note_to_note(uint8_t midi_note)
{
  return (int8_t)midi_note - 24;
}

void play_note(uint8_t channel, uint8_t midi_note)
{
  //   notes[channel] = note;

  uint8_t note = midi_note_to_note(midi_note);
  assigned_notes[channel] = midi_note;

  switch (channel) {
  case CHN_SQ1:
    env1.gate = 1;
    portamento_target_notes[0] = note;
    break;
	
  case CHN_SQ2:
    env2.gate = 1;
    portamento_target_notes[1] = note;
    break;
	
  case CHN_TRI:
    tri.silenced = 0;
    portamento_target_notes[2] = note;
    break;

  case CHN_NOISE:
    env3.gate = 1;
    noise_period = note - 24;
    break;

  case CHN_DMC:
    if (sample_occupied(midi_note - 60)) {
      sample_load(&dmc.sample, midi_note - 60);
      if (dmc.sample.size != 0)
	dmc.sample_enabled = 1;
      break;
    }
  }
}

void stop_note(uint8_t channel)
{
  switch (channel) {
  case CHN_SQ1:
    env1.gate = 0;
    break;
	
  case CHN_SQ2:
    env2.gate = 0;
    break;
	
  case CHN_TRI:
    tri.silenced = 1;
    break;

  case CHN_NOISE:
    env3.gate = 0;
    break;

  case CHN_DMC:
    dmc.sample_enabled = 0;
  }

  assigned_notes[channel] = 0;
}

void assigner_handler(void)
{
    
}
