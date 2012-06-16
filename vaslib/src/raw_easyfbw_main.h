#ifndef FBW_H
#define FBW_H

#include <windows.h>
#include <math.h>
#include "FSData.h"

#define GROUNDMODE 32001
#define FLAREMODE  32002
#define NORMAL     32003

class FBW
	{
	public:
		FBW(FSData &Data, LONG &JoyX, LONG &JoyY);
		~FBW();

		void SetPitchKp(int target) {PC.Kp = target;}
		double GetPitchKp() {return PC.Kp;}

		void SetBankKp(int target) {BC.Kp = target;}
		double GetBankKp() {return BC.Kp;}

		double GetPitchTarget() {return PC.target;}
		double GetBankTarget() {return BC.target;}

		void SetPitchStdRate(double target) {PC.stdrate = target;}
		double GetPitchStdRate() {return PC.stdrate;}

		void SetBankStdRate(double target) {BC.stdrate = target;}
		double GetBankStdRate() {return BC.stdrate;}

		void SetModeBeep(int B) {ModeBeep = B;}
		int GetModeBeep() {return ModeBeep;}

		int GetOverride() {return iOverride;}

		int GetMode() {return Mode;}

		void CalcOutput();
		void CalcMode();

		void FBWToggle();
		bool GetStatus() {return bStatus;}

	private:
		class MFController
			{
			public:
				MFController();
				~MFController();

				//double Update(double target, double y, double yi, double variable, double stdrate, double u_min, double u_max, double Kp, double *u_n, bool *bInit, double (*pSmoothFunc)(double DiffFactor));
				short CalcFBW(const LONG &JoyValue, const double &yi, const short &variable, const double &stdrate, const short &u_min, const short &u_max, const int &Kp, short &u_n, bool &bInit);

				double delta_u_n;		// incremental output
				double difffactor;
			};

		struct PitchController
			{
			double (*pSmoother)(double DiffFactor);

			int Kp;					// Proportional Gain
			short u_min, u_max;		// minimum / maximum value
			short u_n;					// ouput
			double stdrate;				// standard rate
			double target;				// target value
			bool bStable;
			bool bInit;					// Initialization Boolean
			bool FBWInit;
			int iStableTimer;
			};

		struct BankController
			{
			double (*pSmoother)(double DiffFactor);

			int Kp;					// Proportional Gain
			short u_min, u_max;		// minimum / maximum value
			short u_n;					// ouput
			double stdrate;				// standard rate
			double target;				// target value
			bool bStable;
			bool bInit;					// Initialization Boolean
			bool FBWInit;
			int iStableTimer;
			};

		const LONG &lJoyX, &lJoyY;

		DWORD dwLiftOff;

		int Mode;

		bool bStatus;
		bool bFlareInhibit;
		int iOverride;
		int n, i;
		int ModeBeep;

		MFController *pMFC;

		PitchController PC;
		BankController BC;

		FSData &FSdata;

	};

#endif