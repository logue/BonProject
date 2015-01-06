// PtTerTuner.h: PT�n��g�`���[�i�N���X
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


#include "PtBaseTuner.h"



/////////////////////////////////////////////////////////////////////////////
// PT�n��g�`���[�i�N���X
/////////////////////////////////////////////////////////////////////////////

class CPtTerTuner :	public CPtBaseTuner
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CPtTerTuner)

// CPtBaseTuner
	virtual const bool SetChannel(const DWORD dwSpace, const DWORD dwChannel);

// IConfigTarget
	virtual const bool SetProperty(const IConfigProperty *pProperty);

// CPtTerTuner
	CPtTerTuner(IBonObject *pOwner);
	virtual ~CPtTerTuner(void);

protected:
};
