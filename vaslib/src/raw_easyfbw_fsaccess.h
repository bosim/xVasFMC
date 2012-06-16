#ifndef FSDATA_H
#define FSDATA_H

#define OVERRIDE_ELEVATOR 1
#define OVERRIDE_AILERON  2
#define OVERRIDE_ELEVATORTRIM 32

struct FSData
	{
	double dPitch, dBank, dRA, dVS, dPitchVel, dBankVel, dVertAccel;
	bool bAPEngaged, bInstReplay, bSlewMode, bPaused, bOnGround;
	short sAileron, sElevator, sElevatorTrim;
	BYTE bOverride;
	};

#endif