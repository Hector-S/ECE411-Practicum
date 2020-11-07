
## File Descriptions

**411SoundControl_200mW_4v.asc :** Uses a 3.3v analog input and a 4.5v DC supply to provide a 200mW max power output across an 8ohm speaker. Power output can be easily increased if Vin or the BJT gain is lower than expected by increasing the DC power supply above the 2N3904 transistor.

**411SoundControl_DualSupply.asc :** Uses +4 and -4 supply rails for the npn and pnp bjt transistors, respectively. Produces a waveform across the 8ohm speaker with an output up to 1.2W. Does not have the issues with turning off briefly at 0v which the single supply amplifier has (unlikely to be an issue due to only needing to produce 16 tones).

**411SoundControl_270mW_9v.asc :** Design to be prototyped. Produces 270mW w/ 9v DC supply across 8ohm speaker. Final power output likely to be lower due to ambitious default LTspice beta values. Gain can be altered for final design on pcb by changing R1 and R4 values (should be equivalent to not clip waveform). R7 to be potentiometer to set volume.
