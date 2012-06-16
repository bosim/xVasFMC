#include "FBW.h"

double PitchSmoother(double DiffFactor)
{
	if(DiffFactor >0.0)
    {
		if(DiffFactor > 8.0) return 1.5;
		else if(DiffFactor > 4.0) return 0.8;
		else if(DiffFactor > 3.0) return 0.4;
		else if(DiffFactor > 2.0) return 0.2;
		else if(DiffFactor > 0.5) return 0.1;
		else if(DiffFactor > 0.25) return 0.05;
		else return 0.01;
    }
	else
    {
		if(DiffFactor < -8.0) return -1.5;
		else if(DiffFactor < -4.0) return -0.8;
		else if(DiffFactor < -3.0) return -0.4;
		else if(DiffFactor < -2.0) return -0.2;
		else if(DiffFactor < -0.5) return -0.1;
		else if(DiffFactor < -0.25) return -0.05;
		else return -0.01;
    }
}

double BankSmoother(double DiffFactor)
{
	if(DiffFactor >0.0)
    {
		if(DiffFactor > 5.0) return 1.5;
		else if(DiffFactor > 1.0) return 0.5;
		else if(DiffFactor > 0.1) return 0.1;
		else return 0.01;
    }
	else
    {
		if(DiffFactor < -5.0) return -1.5;
		else if(DiffFactor < -1.0) return -0.5;
		else if(DiffFactor < -0.1) return -0.1;
		else return -0.01;
    }
}

FBW::FBW(FSData &Data, LONG &JoyX, LONG &JoyY) :
	FSdata(Data),
	lJoyX(JoyX),
	lJoyY(JoyY)
{
	pMFC = new MFController();
	ModeBeep = 1;
	bStatus = false;
	bFlareInhibit = true;

	n = i = 0;

	Mode = GROUNDMODE;

	//PC.Kp = 125000.; B767
	PC.pSmoother = PitchSmoother;

	PC.Kp = 10000;

	PC.stdrate = 0.035;

	PC.u_min = -16383;
	PC.u_max = 16383;

	PC.u_n = 0;

	PC.target = 0.0;

	PC.bInit = false;

	PC.FBWInit = false;

	PC.bStable = false;

	PC.iStableTimer = 0;

	//Bank Controller

	BC.pSmoother = BankSmoother;

	BC.Kp = 5000;

	BC.stdrate = 0.05;

	BC.u_min = -16383;
	BC.u_max = 16383;

	BC.u_n = 0;

	BC.target = 0.0;

	BC.bInit = false;

	BC.FBWInit = false;

	BC.bStable = false;

	BC.iStableTimer = 0;

	dwLiftOff = 0;
}

FBW::~FBW()
{
	delete pMFC;
}

void FBW::FBWToggle()
{
	if(bStatus)
    {
		bStatus = false;
		Mode = GROUNDMODE;
		PC.bInit = false;
		PC.FBWInit = false;
		PC.bStable = false;
		PC.iStableTimer = 0;
		PC.target = 0;

		BC.bInit = false;
		BC.FBWInit = false;
		BC.bStable = false;
		BC.iStableTimer = 0;
		BC.target = 0;

		FSdata.bOverride = 0;
    }
	else bStatus = true;
}

void FBW::CalcMode()
{
	int OldMode = Mode;
	if(FSdata.dRA > 55)
    {
		if(Mode != GROUNDMODE) Mode = NORMAL;
    }
	else if(FSdata.dRA <= 55 && !bFlareInhibit && FSdata.dVS < 0.)
    {
		Mode = FLAREMODE;
    }
	if(FSdata.bOnGround) Mode = GROUNDMODE;

	if(Mode == GROUNDMODE)
    {
		bFlareInhibit = true;
		if(FSdata.bOnGround)
        {
			dwLiftOff = GetTickCount();
        }
		else
        {
			if((GetTickCount() - dwLiftOff) > 3000)
            {
				Mode = NORMAL;
            }
        }
    }
	else if(Mode == NORMAL)
    {
		bFlareInhibit = false;
    }

	if(FSdata.bAPEngaged)
    {
		PC.target = FSdata.dPitch;
		BC.target = FSdata.dBank;
    }
	if(ModeBeep == 1)
    {
		if(OldMode != Mode && bStatus) PlaySound("Confirmation.wav", NULL, SND_ASYNC);
    }
}

void FBW::CalcOutput()
{
	if(!bStatus) return;

	switch(Mode)
    {
		case GROUNDMODE:
			PC.bInit = PC.FBWInit = false;
			PC.target = FSdata.dPitch;
			BC.bInit = BC.FBWInit = false;
			BC.target = FSdata.dBank;
			FSdata.bOverride = 0;
			i = 0;
			break;

		case FLAREMODE:
			PC.bInit = PC.FBWInit = false;
			PC.target = FSdata.dPitch;
			BC.bInit = BC.FBWInit = false;
			BC.target = FSdata.dBank;
			FSdata.bOverride = OVERRIDE_ELEVATORTRIM;
			if(i<=10)
            {
				if(i%2 == 0)
                {
					FSdata.sElevatorTrim -= 48;
                }
				i++;
            }
			break;

		case NORMAL:
			FSdata.bOverride = OVERRIDE_AILERON | OVERRIDE_ELEVATOR | OVERRIDE_ELEVATORTRIM;
			i = 0;

			if(FSdata.sElevator > 100)
            {
				FSdata.sElevatorTrim += 6;
            }
			else if(FSdata.sElevator < -100)
            {
                FSdata.sElevatorTrim -= 6;
            }

			//PITCH MODE

			if(labs(lJoyY) < 20)
            {
				//Maintain pitch, but only after stabilization
				//PC.bInit = false;
				FSdata.sElevator = (pMFC->CalcFBW(0, -FSdata.dPitchVel, FSdata.sElevator, 0.0, PC.u_min, PC.u_max, PC.Kp, PC.u_n, PC.FBWInit));
            }
			else
            {
				//Reset Pitch Controller
				//PC.bInit = false;
				//Sidestick input = acceleration
				FSdata.sElevator = (pMFC->CalcFBW(-(LONG)(lJoyY*0.5), -FSdata.dPitchVel, FSdata.sElevator, -PC.stdrate, PC.u_min, PC.u_max, PC.Kp, PC.u_n, PC.FBWInit));
            }

			//ROLL MODE

			if(labs(lJoyX) < 20)
            {
				//BC.bInit = false;

				//Stabilize roll -> acceleration = 0
                FSdata.sAileron = (pMFC->CalcFBW(0, 
                                                 -FSdata.dBankVel, 
                                                 FSdata.sAileron, 
                                                 0, 
                                                 BC.u_min, 
                                                 BC.u_max, 
                                                 BC.Kp, 
                                                 BC.u_n, 
                                                 BC.FBWInit));
            }
			else
            {
				//Reset stabilization

				//Reset Pitch Controller
				//BC.bInit = false;

				//Sidestick input = acceleration
				FSdata.sAileron = (pMFC->CalcFBW(-(LONG)(lJoyX*0.5), 
                                                 -FSdata.dBankVel, 
                                                 FSdata.sAileron, 
                                                 -BC.stdrate, 
                                                 BC.u_min, 
                                                 BC.u_max, 
                                                 BC.Kp, 
                                                 BC.u_n, 
                                                 BC.FBWInit));
				//FSdata.sAileron = lJoyX;
            }
			break;
    }
}


FBW::MFController::MFController()
{
	difffactor = 0.0;
	delta_u_n = 0.0;
}

FBW::MFController::~MFController()
{
}

//(-pMFC->Update(PC.target, pIPC->GetPitch(), pIPC->GetPitchVel(), pIPC->GetElevator(), PC.stdrate, PC.u_min, PC.u_max, PC.Kp, PC.u_n, PC.bInit, PC.pSmoother)
/*double FBW::MFController::Update(double target, double y, double yi, double variable, double stdrate, double u_min, double u_max, double Kp, double *u_n, bool *bInit, double (*pSmoothFunc)(double DiffFactor))
  {
  if(!*bInit)
  {
  *bInit = true;
  *u_n = variable;
  }
  else
  {

  difffactor = (*pSmoothFunc)(target - y);

  delta_u_n = ((stdrate * difffactor) - yi) * Kp;

  *u_n = (*u_n + delta_u_n);

  if(*u_n > u_max) *u_n = u_max;
  else if(*u_n < u_min) *u_n = u_min;

  }

  return *u_n;
  }*/

short FBW::MFController::CalcFBW(const LONG &StickValue, 
                                 const double &yi, const short &variable, 
                                 const double &stdrate, 
                                 const short &u_min, 
                                 const short &u_max, 
                                 const int &Kp, 
                                 short &u_n, 
                                 bool &bInit)
{
	if(!bInit)
    {
		bInit = true;
		u_n = variable;
    }
	else
    {
		short delta_u_n = (short)(((stdrate * StickValue *0.001) - yi) * Kp);

		u_n = (u_n + delta_u_n);

		if(u_n > u_max) u_n = u_max;
		else if(u_n < u_min) u_n = u_min;

    }

	return u_n;
}
