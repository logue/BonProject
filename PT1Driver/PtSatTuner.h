// PtSatTuner.h: PT BS/110CS�`���[�i�N���X
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


#include "PtBaseTuner.h"



/////////////////////////////////////////////////////////////////////////////
// PT BS/110CS�`���[�i�N���X
/////////////////////////////////////////////////////////////////////////////

class CPtSatTuner :	public CPtBaseTuner
{
public:
// CBonObject
	DECLARE_IBONOBJECT(CPtSatTuner)

// CPtBaseTuner
	virtual const bool SetChannel(const DWORD dwSpace, const DWORD dwChannel);

// IConfigTarget
	virtual const bool SetProperty(const IConfigProperty *pProperty);

// CPtSatTuner
	CPtSatTuner(IBonObject *pOwner);
	virtual ~CPtSatTuner(void);

protected:
};
