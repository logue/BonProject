// PtDriverIf.h: PT1�h���C�o�J�X�^���C���^�t�F�[�X
//
/////////////////////////////////////////////////////////////////////////////

#pragma once


/////////////////////////////////////////////////////////////////////////////
// PT1�`���[�i�v���p�e�B�C���^�t�F�[�X
/////////////////////////////////////////////////////////////////////////////

class IPtBaseTunerProperty :	public IBonObject
{
public:
	bool m_bDefaultLnbPower;	// LNB�d���f�t�H���g��Ԑݒ�
	bool m_bEnaleVHF;			// VHF  �`���[�j���O��ԗL��
	bool m_bEnaleCATV;			// CATV �`���[�j���O��ԗL��
	bool m_bEnaleCS;			// 110CS�`���[�j���O��ԗL��
	bool m_bEnaleBS;			// BSF  �`���[�j���O��ԗL��
};
