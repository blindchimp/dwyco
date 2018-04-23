
#define START 0
#define CTS_WAIT 1
#define PLAY_INCOMING 2
#define PLAY_OFF_RECORD 3

#define MUTE_OFF 0
#define MUTE_ON 1
#define CTS_OFF 2
#define CTS_ON 3

chan gAB = [8] of {byte};
chan gBA = [8] of {byte};
chan local_sig = [8] of {byte};

proctype half_duplex(chan AB; chan BA)
{
byte state;
byte cts;
bool entry;

atomic {
cts = 0;
AB!CTS_OFF;
state = START;
entry = 0;
}


do
:: (entry == 0) ->
	if
	:: state == START ; BA?CTS_OFF -> atomic{state = CTS_WAIT; entry = 1}
	:: (state == START) ; BA?CTS_ON -> atomic{state = PLAY_INCOMING; entry = 1;}
	:: (state == CTS_WAIT) ; BA?CTS_ON -> atomic{state = PLAY_INCOMING; entry = 1;}
	:: (state == CTS_WAIT) ; BA?CTS_OFF ; skip;
	:: (state == PLAY_INCOMING) ; BA?CTS_OFF ->atomic{state = CTS_WAIT; entry = 1;}
	:: (state == PLAY_INCOMING) ; BA?CTS_ON ; skip;
	:: state == PLAY_OFF_RECORD ; BA?CTS_OFF ->atomic{state = CTS_WAIT; entry = 1;}
	:: state == PLAY_OFF_RECORD ; BA?CTS_ON; skip;
	/* this is a button push, essentially random input */
	:: (state == PLAY_INCOMING) -> atomic{state = PLAY_OFF_RECORD; entry = 1;}
	/* button release, random input */
	:: (state == PLAY_OFF_RECORD) -> atomic{state = PLAY_INCOMING; entry = 1;}
	fi
:: (entry == 1) -> 
	if
	:: state == CTS_WAIT -> atomic { AB!CTS_ON;cts = 1; entry = 0;}
	:: state == PLAY_INCOMING -> atomic { AB!CTS_ON; cts = 1; entry = 0;}
	:: state == PLAY_OFF_RECORD -> atomic {AB!CTS_OFF; cts = 1; entry = 0;}
	fi
od


}

init {
run half_duplex(gAB, gBA);
run half_duplex(gBA, gAB);

}
