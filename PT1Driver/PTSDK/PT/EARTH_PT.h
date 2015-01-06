// ========================================================================================
//	EARTH_PT.h
//	version 1.0 (2008.10.25)
// ========================================================================================

#ifndef _EARTH_PT_H
#define _EARTH_PT_H

#include "Prefix.h"

namespace EARTH {
namespace PT {
	class Device;
	class Device2;
	class ROM;

	// +------------+
	// | �o�X�N���X |
	// +------------+
	// �o�X��̃f�o�C�X��񋓂��܂��B�܂��f�o�C�X�C���X�^���X�𐶐����܂��B
	class Bus {
	public:
		typedef status (*NewBusFunction)(Bus **);

		// [�@�\] �C���X�^���X�����
		// [����] delete �͎g���܂���B���̊֐����Ăяo���Ă��������B
		virtual status Delete() = 0;

		// [�@�\] �\�t�g�E�F�A�o�[�W�������擾
		// [����] �o�[�W������ 1.2.3 �̏ꍇ�A�l�� 0x123 �ɂȂ�܂��B
		//        ��� 24 �r�b�g�������ł���΃o�C�i���݊��ɂȂ�悤�ɓw�߂܂��̂ŁA
		//        ((version >> 8) == 1) �ł��邩���`�F�b�N���Ă��������B 
		virtual status GetVersion(uint *version) const = 0;

		// �f�o�C�X���
		struct DeviceInfo {
			uint Bus, Slot, Function;	// PCI �o�X��̈ʒu
			uint BadBitCount;			// PCI �o�X�̃r�b�g������
		};

		// [�@�\] �F������Ă���f�o�C�X�̃��X�g���擾
		// [����] PCI �o�X���X�L�������Ĉȉ��̏�����S�Ė������f�o�C�X�����X�g�A�b�v���܂��B
		//        �x���_ID: 0x10ee / �f�o�C�XID: 0x211a / �T�u�V�X�e���x���_ID: ~0x10ee / �T�u�V�X�e��ID: ~0x211a
		//        
		//        �X���b�g�ƃ{�[�h�[�q�̐ڐG�������ꍇ�A������ ID �Ƀr�b�g�����������邱�Ƃ�����܂��B
		//        ���̂悤�ȏ󋵂ł��f�o�C�X�����o�ł���悤�ɁAmaxBadBitCount �Ńr�b�g�����̋��e������w�肷�邱�Ƃ��ł��܂��B
		//        64�r�b�g(16�r�b�g�~4) �̊e�r�b�g���r���A����r�b�g���� maxBadBitCount �ȉ��̃f�o�C�X�����X�g�A�b�v���܂��B
		//        
		//        maxBadBitCount �� 3 �ȉ��̒l���w�肵�܂��B
		//        DeviceInfo::BadBitCount �� 0 �łȂ��f�o�C�X�� Device::Open() ���邱�Ƃ͂ł��܂���B
		virtual status Scan(DeviceInfo *deviceInfoPtr, uint *deviceInfoCount, uint maxBadBitCount = 0) = 0;

		// [�@�\] �f�o�C�X�C���X�^���X�𐶐�����
		// [����] �f�o�C�X���\�[�X�̔r���`�F�b�N�͂��̊֐��ł͍s���܂���BDevice::Open() �ōs���܂��B
		//        Device2 �͔���J�C���^�[�t�F�[�X�ł��Bdevice2 �� NULL �ɂ��Ă��������B
		virtual status NewDevice(const DeviceInfo *deviceInfoPtr, Device **device, Device2 **device2 = NULL) = 0;

	protected:
		virtual ~Bus() {}
	};

	// +----------------+
	// | �f�o�C�X�N���X |
	// +----------------+
	// ���̃C���X�^���X 1 ���{�[�h 1 ���ɑΉ����Ă��܂��B
	class Device {
	public:
		// ----
		// ���
		// ----

		// [�@�\] �C���X�^���X�����
		// [����] delete �͎g���܂���B���̊֐����Ăяo���Ă��������B
		virtual status Delete() = 0;
		
		// ------------------
		// �I�[�v���E�N���[�Y
		// ------------------

		// [�@�\] �f�o�C�X�̃I�[�v���E�N���[�Y
		// [����] �f�o�C�X�̃I�[�v���͈ȉ��̎菇�ɉ����čs���܂��B
		//        1. ���Ƀf�o�C�X���I�[�v������Ă��Ȃ������m�F����B
		//        2. ���r�W����ID (�R���t�B�M�����[�V������� �A�h���X 0x08) �� 0x01 �ł��邩�𒲂ׂ�B
		//        3. �R���t�B�M�����[�V������Ԃ̃f�o�C�X�ŗL���W�X�^�̈���g���� PCI �o�X�ł̃r�b�g�������Ȃ������m�F����B
		//        4. ���� SDK �Ő��䂪�\�� FPGA ��H�̃o�[�W�����ł��邩���m�F����B

		virtual status Open () = 0;
		virtual status Close() = 0;

		// --------------------------------------
		// PCI �N���b�N�J�E���^�E���C�e���V�^�C�}
		// --------------------------------------

		// [�@�\] PCI �N���b�N�J�E���^���擾
		// [����] �J�E���^���� 32 �r�b�g�ł��B0xffffffff �̎��� 0 �ɂȂ�܂��B
		virtual status GetPciClockCounter(uint *counter) = 0;

		// [�@�\] PCI ���C�e���V�^�C�}�l�̐ݒ�E�擾
		// [����] ���� 3 �r�b�g�͎�������Ă��Ȃ����߁A�擾�����l�� 8 �̔{���ɂȂ�܂��B
		virtual status SetPciLatencyTimer(byte  latencyTimer)       = 0;
		virtual status GetPciLatencyTimer(byte *latencyTimer) const = 0;

		// ------------
		// �d���E������
		// ------------

		enum LnbPower {
			LNB_POWER_OFF,	// �I�t
			LNB_POWER_15V,	// 15V �o��
			LNB_POWER_11V	// 11V �o�� (���m�ɂ� PCI �X���b�g�� +12V ���� 0.6V ���x���������l)
		};

		// [�@�\] LNB �d������
		// [����] �`���[�i�[�̓d���Ƃ͓Ɨ��ɐ���\�ł��B�f�t�H���g�l�� LNB_POWER_OFF �ł��B

		virtual status SetLnbPower(LnbPower  lnbPower)       = 0;
		virtual status GetLnbPower(LnbPower *lnbPower) const = 0;

		// [�@�\] �f�o�C�X���N���[�Y�i�ُ�I���ɂƂ��Ȃ��N���[�Y���܂ށj���� LNB �d������
		// [����] �f�t�H���g�l�� LNB_POWER_OFF �ł��B

		virtual status SetLnbPowerWhenClose(LnbPower  lnbPower)       = 0;
		virtual status GetLnbPowerWhenClose(LnbPower *lnbPower) const = 0;

		// [�@�\] �`���[�i�[�d���E�n�[�h�E�F�A���Z�b�g����
		// [����] TUNER_POWER_ON_RESET_ENABLE ���� TUNER_POWER_ON_RESET_DISABLE �̑J�ڂɂ͍Œ� 15ms �̑҂����Ԃ��K�v�ł��B
		enum TunerPowerReset {				// �d���^�n�[�h�E�F�A���Z�b�g
			TUNER_POWER_OFF,				// �I�t�^�C�l�[�u��
			TUNER_POWER_ON_RESET_ENABLE,	// �I���^�C�l�[�u��
			TUNER_POWER_ON_RESET_DISABLE	// �I���^�f�B�Z�[�u��
		};
		virtual status SetTunerPowerReset(TunerPowerReset  tunerPowerReset)       = 0;
		virtual status GetTunerPowerReset(TunerPowerReset *tunerPowerReset) const = 0;

		// [�@�\] �`���[�i�[������
		// [����] SetTunerPowerReset(TUNER_POWER_ON_RESET_DISABLE) ����Œ� 1��s �o�ߌ�� 1 �񂾂��Ăяo���܂��B 
		virtual status InitTuner(uint tuner) = 0;

		// ��M����
		enum ISDB {
			ISDB_S,
			ISDB_T,

			ISDB_COUNT
		};

		// [�@�\] �`���[�i�[�ȓd�͐���
		// [����] ����IC �Ɖq���� PLL-IC �݂̂��Ώۂł��B�`���[�i�[���j�b�g���̑��̉�H�� SetTunerPowerReset(TUNER_POWER_OFF)
		//        �Ƃ��Ȃ�����A�d�͂���������܂��B����IC �̏���d�͂̓`���[�i�[���W���[���� 15% �ł��B
		virtual status SetTunerSleep(uint tuner, ISDB isdb, bool  sleep)       = 0;
		virtual status GetTunerSleep(uint tuner, ISDB isdb, bool *sleep) const = 0;

		// ----------
		// �ǔ����g��
		// ----------

		// [�@�\] �ǔ����g���̐���
		// [����] offset �Ŏ��g���̒������\�ł��B�P�ʂ� ISDB-S �̏ꍇ�� 1MHz�AISDB-T �̏ꍇ�� 1/7MHz �ł��B
		//        �Ⴆ�΁AC24 ��W����� 2MHz �������g���ɐݒ肷��ɂ� SetFrequency(tuner, ISDB_T, 23, 7*2) �Ƃ��܂��B
		virtual status SetFrequency(uint tuner, ISDB isdb, uint  channel, int  offset = 0)       = 0;
		virtual status GetFrequency(uint tuner, ISDB isdb, uint *channel, int *offset = 0) const = 0;

		// (ISDB-S)
		// PLL ���g���X�e�b�v�� 1MHz �̂��߁A���ۂɐݒ肳�����g���� f' �ɂȂ�܂��B
		// +----+------+---------+---------+ +----+------+---------+---------+ +----+------+---------+---------+
		// | ch | TP # | f (MHz) | f'(MHz) | | ch | TP # | f (MHz) | f'(MHz) | | ch | TP # | f (MHz) | f'(MHz) |
		// +----+------+---------+---------+ +----+------+---------+---------+ +----+------+---------+---------+
		// |  0 | BS 1 | 1049.48 | 1049.00 | | 12 | ND 2 | 1613.00 | (����)  | | 24 | ND 1 | 1593.00 | (����)  |
		// |  1 | BS 3 | 1087.84 | 1088.00 | | 13 | ND 4 | 1653.00 | (����)  | | 25 | ND 3 | 1633.00 | (����)  |
		// |  2 | BS 5 | 1126.20 | 1126.00 | | 14 | ND 6 | 1693.00 | (����)  | | 26 | ND 5 | 1673.00 | (����)  |
		// |  3 | BS 7 | 1164.56 | 1165.00 | | 15 | ND 8 | 1733.00 | (����)  | | 27 | ND 7 | 1713.00 | (����)  |
		// |  4 | BS 9 | 1202.92 | 1203.00 | | 16 | ND10 | 1773.00 | (����)  | | 28 | ND 9 | 1753.00 | (����)  |
		// |  5 | BS11 | 1241.28 | 1241.00 | | 17 | ND12 | 1813.00 | (����)  | | 29 | ND11 | 1793.00 | (����)  |
		// |  6 | BS13 | 1279.64 | 1280.00 | | 18 | ND14 | 1853.00 | (����)  | | 30 | ND13 | 1833.00 | (����)  |
		// |  7 | BS15 | 1318.00 | (����)  | | 19 | ND16 | 1893.00 | (����)  | | 31 | ND15 | 1873.00 | (����)  |
		// |  8 | BS17 | 1356.36 | 1356.00 | | 20 | ND18 | 1933.00 | (����)  | | 32 | ND17 | 1913.00 | (����)  |
		// |  9 | BS19 | 1394.72 | 1395.00 | | 21 | ND20 | 1973.00 | (����)  | | 33 | ND19 | 1953.00 | (����)  |
		// | 10 | BS21 | 1433.08 | 1433.00 | | 22 | ND22 | 2013.00 | (����)  | | 34 | ND21 | 1993.00 | (����)  |
		// | 11 | BS23 | 1471.44 | 1471.00 | | 23 | ND24 | 2053.00 | (����)  | | 35 | ND23 | 2033.00 | (����)  |
		// +----+------+---------+---------+ +----+------+---------+---------+ +----+------+---------+---------+
		// 
		// (ISDB-T)
		// +-----+-----+---------+ +-----+-----+---------+ +-----+-----+---------+ +-----+-----+---------+ +-----+-----+---------+
		// | ch. | Ch. | f (MHz) | | ch. | Ch. | f (MHz) | | ch. | Ch. | f (MHz) | | ch. | Ch. | f (MHz) | | ch. | Ch. | f (MHz) |
		// +-----+-----+---------+ +-----+-----+---------+ +-----+-----+---------+ +-----+-----+---------+ +-----+-----+---------+
		// |   0 |   1 |  93+1/7 | |  23 | C24 | 231+1/7 | |  46 | C47 | 369+1/7 | |  69 |  19 | 509+1/7 | |  92 |  42 | 647+1/7 |
		// |   1 |   2 |  99+1/7 | |  24 | C25 | 237+1/7 | |  47 | C48 | 375+1/7 | |  70 |  20 | 515+1/7 | |  93 |  43 | 653+1/7 |
		// |   2 |   3 | 105+1/7 | |  25 | C26 | 243+1/7 | |  48 | C49 | 381+1/7 | |  71 |  21 | 521+1/7 | |  94 |  44 | 659+1/7 |
		// |   3 | C13 | 111+1/7 | |  26 | C27 | 249+1/7 | |  49 | C50 | 387+1/7 | |  72 |  22 | 527+1/7 | |  95 |  45 | 665+1/7 |
		// |   4 | C14 | 117+1/7 | |  27 | C28 | 255+1/7 | |  50 | C51 | 393+1/7 | |  73 |  23 | 533+1/7 | |  96 |  46 | 671+1/7 |
		// |   5 | C15 | 123+1/7 | |  28 | C29 | 261+1/7 | |  51 | C52 | 399+1/7 | |  74 |  24 | 539+1/7 | |  97 |  47 | 677+1/7 |
		// |   6 | C16 | 129+1/7 | |  29 | C30 | 267+1/7 | |  52 | C53 | 405+1/7 | |  75 |  25 | 545+1/7 | |  98 |  48 | 683+1/7 |
		// |   7 | C17 | 135+1/7 | |  30 | C31 | 273+1/7 | |  53 | C54 | 411+1/7 | |  76 |  26 | 551+1/7 | |  99 |  49 | 689+1/7 |
		// |   8 | C18 | 141+1/7 | |  31 | C32 | 279+1/7 | |  54 | C55 | 417+1/7 | |  77 |  27 | 557+1/7 | | 100 |  50 | 695+1/7 |
		// |   9 | C19 | 147+1/7 | |  32 | C33 | 285+1/7 | |  55 | C56 | 423+1/7 | |  78 |  28 | 563+1/7 | | 101 |  51 | 701+1/7 |
		// |  10 | C20 | 153+1/7 | |  33 | C34 | 291+1/7 | |  56 | C57 | 429+1/7 | |  79 |  29 | 569+1/7 | | 102 |  52 | 707+1/7 |
		// |  11 | C21 | 159+1/7 | |  34 | C35 | 297+1/7 | |  57 | C58 | 435+1/7 | |  80 |  30 | 575+1/7 | | 103 |  53 | 713+1/7 |
		// |  12 | C22 | 167+1/7 | |  35 | C36 | 303+1/7 | |  58 | C59 | 441+1/7 | |  81 |  31 | 581+1/7 | | 104 |  54 | 719+1/7 |
		// |  13 |   4 | 173+1/7 | |  36 | C37 | 309+1/7 | |  59 | C60 | 447+1/7 | |  82 |  32 | 587+1/7 | | 105 |  55 | 725+1/7 |
		// |  14 |   5 | 179+1/7 | |  37 | C38 | 315+1/7 | |  60 | C61 | 453+1/7 | |  83 |  33 | 593+1/7 | | 106 |  56 | 731+1/7 |
		// |  15 |   6 | 185+1/7 | |  38 | C39 | 321+1/7 | |  61 | C62 | 459+1/7 | |  84 |  34 | 599+1/7 | | 107 |  57 | 737+1/7 |
		// |  16 |   7 | 191+1/7 | |  39 | C40 | 327+1/7 | |  62 | C63 | 465+1/7 | |  85 |  35 | 605+1/7 | | 108 |  58 | 743+1/7 |
		// |  17 |   8 | 195+1/7 | |  40 | C41 | 333+1/7 | |  63 |  13 | 473+1/7 | |  86 |  36 | 611+1/7 | | 109 |  59 | 749+1/7 |
		// |  18 |   9 | 201+1/7 | |  41 | C42 | 339+1/7 | |  64 |  14 | 479+1/7 | |  87 |  37 | 617+1/7 | | 110 |  60 | 755+1/7 |
		// |  19 |  10 | 207+1/7 | |  42 | C43 | 345+1/7 | |  65 |  15 | 485+1/7 | |  88 |  38 | 623+1/7 | | 111 |  61 | 761+1/7 |
		// |  20 |  11 | 213+1/7 | |  43 | C44 | 351+1/7 | |  66 |  16 | 491+1/7 | |  89 |  39 | 629+1/7 | | 112 |  62 | 767+1/7 |
		// |  21 |  12 | 219+1/7 | |  44 | C45 | 357+1/7 | |  67 |  17 | 497+1/7 | |  90 |  40 | 635+1/7 | +-----+-----+---------+
		// |  22 | C23 | 225+1/7 | |  45 | C46 | 363+1/7 | |  68 |  18 | 503+1/7 | |  91 |  41 | 641+1/7 |
		// +-----+-----+---------+ +-----+-----+---------+ +-----+-----+---------+ +-----+-----+---------+
		// 
		// C24�`C27 �́A�P�[�u���e���r�ǂɂ�艺�L�̎��g���ő��M����Ă���ꍇ������܂��B
		// +-----+---------+
		// | Ch. | f (MHz) |
		// +-----+---------+
		// | C24 | 233+1/7 |
		// | C25 | 239+1/7 |
		// | C26 | 245+1/7 |
		// | C27 | 251+1/7 |
		// +-----+---------+

		// ----------
		// ���g���덷
		// ----------

		// [�@�\] ���g���덷���擾
		// [����] �l�̈Ӗ��͎��̒ʂ�ł��B
		//        �N���b�N���g���덷: clock/100 (ppm)
		//        �L�����A���g���덷: carrier (Hz)
		//        �����g�̎��g�����x�͏\���ɍ������肷��ƁA�덷����������v�f�Ƃ��Ĉȉ��̂悤�Ȃ��̂��l�����܂��B
		//        (ISDB-S) LNB �ł̎��g���ϊ����x / �q���� PLL-IC �ɐڑ�����Ă���U���q�̐��x / ���� IC �ɐڑ�����Ă���U���q�̐��x
		//        (ISDB-T) �n�㑤 PLL-IC �ɐڑ�����Ă���U���q�̐��x / ���� IC �ɐڑ�����Ă���U���q�̐��x
		virtual status GetFrequencyOffset(uint tuner, ISDB isdb, int *clock, int *carrier) = 0;

		// --------
		// C/N�EAGC
		// --------

		// [�@�\] C/N �� AGC ���擾
		// [����] C/N �͒Z���C�e���V�ő���ł��邽�߁A�A���e�i�̌����𒲐�����̂ɕ֗��ł��B
		//        �l�̈Ӗ��͎��̒ʂ�ł��B
		//        C/N                : cn100/100 (dB)
		//        ���݂� AGC �l      : currentAgc
		//        �����ő厞�� AGC �l: maxAgc
		//        currentAgc �͈̔͂� 0 ���� maxAgc �܂łł��B
		virtual status GetCnAgc(uint tuner, ISDB isdb, uint *cn100, uint *currentAgc, uint *maxAgc) = 0;

		// -------------------
		// TS-ID (ISDB-S �̂�)
		// -------------------

		// [�@�\] TS-ID ��ݒ�
		// [����] �ݒ�l������IC �̓���ɔ��f�����܂Ŏ��Ԃ��|����܂��B
		//        GetLayerS() ���Ăяo���O�ɁAGetIdS() ���g���Đ؂�ւ��������������Ƃ��m�F���Ă��������B
		virtual status SetIdS(uint tuner, uint id) = 0;

		// [�@�\] ���ݏ������� TS-ID ���擾
		// [����] GetLayerS() �Ŏ擾�ł��郌�C�����́A���̊֐��Ŏ������ TS-ID �̂��̂ɂȂ�܂��B
		virtual status GetIdS(uint tuner, uint *id) = 0;

		// ------------
		// �G���[���[�g
		// ------------

		// �K�w�C���f�b�N�X
		enum LayerIndex {
			// ISDB-S
			LAYER_INDEX_L = 0,	// ��K�w
			LAYER_INDEX_H,		// ���K�w

			// ISDB-T
			LAYER_INDEX_A = 0,	// A �K�w
			LAYER_INDEX_B,		// B �K�w
			LAYER_INDEX_C		// C �K�w
		};

		// �K�w��
		enum LayerCount {
			// ISDB-S
			LAYER_COUNT_S = LAYER_INDEX_H + 1,

			// ISDB-T
			LAYER_COUNT_T = LAYER_INDEX_C + 1
		};

		// [�@�\] ���[�h�\�����������Œ������ꂽ�G���[���[�g���擾
		// [����] ����Ɏ��Ԃ��|����܂����A��M�i���𐳊m�ɔc������ɂ� C/N �ł͂Ȃ����̃G���[���[�g���Q�l�ɂ��Ă��������B
		//        �ЂƂ̖ڈ��Ƃ��� 2�~10^-4 �ȉ��ł���΁A���[�h�\������������ɂقڃG���[�t���[�ɂȂ�Ƃ����Ă��܂��B
		//        �G���[���[�g�̏W�v�P�ʂ͎��̒ʂ�ł��B
		//        ISDB-S: 1024 �t���[��
		//        ISDB-T: 32 �t���[�� (���[�h 1,2) / 8 �t���[�� (���[�h 3)
		struct ErrorRate {
			uint Numerator, Denominator;
		};
		virtual status GetCorrectedErrorRate(uint tuner, ISDB isdb, LayerIndex layerIndex, ErrorRate *errorRate) = 0;

		// [�@�\] ���[�h�\�����������Œ������ꂽ�G���[���[�g���v�Z���邽�߂̃G���[�J�E���^��������
		// [����] �S�K�w�̃J�E���^�����������܂��B����̊K�w�̃J�E���^�����Z�b�g���邱�Ƃ͂ł��܂���B
		virtual status ResetCorrectedErrorCount(uint tuner, ISDB isdb) = 0;

		// [�@�\] ���[�h�\�����������Œ���������Ȃ����� TS �p�P�b�g�����擾
		// [����] ����24�r�b�g�̂ݗL���ł��i��H�K�͂����������邽�߉�H�ԍ�01 �ɂăr�b�g�����k���j�B
		//        0x??ffffff �̎��� 0x??000000 �ɂȂ�܂��B
		//        TS �p�P�b�g�� 2nd Byte MSB �𐔂��Ă��������l�ɂȂ�܂��B
		//        ���̃J�E���^�� DMA �]���J�n���ɏ���������܂��B
		virtual status GetErrorCount(uint tuner, ISDB isdb, uint *count) = 0;

		// --------------------------
		// TMCC�E���C���[�E���b�N����
		// --------------------------

		// ISDB-S TMCC ���
		// (�Q�l) STD-B20 2.9 TMCC���̍\�� �` 2.11 TMCC���̍X�V
		struct TmccS {
			uint Indicator;		// �ύX�w�� (5�r�b�g)
			uint Mode[4];		// �`�����[�hn (4�r�b�g)
			uint Slot[4];		// �`�����[�hn�ւ̊����X���b�g�� (6�r�b�g)
								// [����TS�^�X���b�g���͎擾�ł��܂���]
			uint Id[8];			// ����TS�ԍ�n�ɑ΂���TS ID (16�r�b�g)
			uint Emergency;		// �N������M�� (1�r�b�g)
			uint UpLink;		// �A�b�v�����N������ (4�r�b�g)
			uint ExtFlag;		// �g���t���O (1�r�b�g)
			uint ExtData[2];	// �g���̈� (61�r�b�g)
		};

		// [�@�\] ISDB-S �� TMCC �����擾
		virtual status GetTmccS(uint tuner, TmccS *tmcc) = 0;

		// ISDB-S �K�w���
		struct LayerS {
			uint Mode [LAYER_COUNT_S];	// �`�����[�h (3�r�b�g) 
			uint Count[LAYER_COUNT_S];	// �_�~�[�X���b�g���܂߂������X���b�g�� (6�r�b�g)
		};

		// [�@�\] ISDB-S �̃��C�������擾
		virtual status GetLayerS(uint tuner, LayerS *) = 0;

		// ISDB-T TMCC ���
		// (�Q�l) STD-B31 3.15.6 TMCC��� �` 3.15.6.8 �Z�O�����g��
		struct TmccT {
			uint System;					// �V�X�e������ (2�r�b�g)
			uint Indicator;					// �`���p�����[�^�؂�ւ��w�W (4�r�b�g)
			uint Emergency;					// �ً}�x������p�N���t���O (1�r�b�g)
											// �J�����g���
			uint Partial;					// ������M�t���O (1�r�b�g)
											// �K�w���
			uint Mode      [LAYER_COUNT_T];	// �L�����A�ϒ����� (3�r�b�g)
			uint Rate      [LAYER_COUNT_T];	// �􍞂ݕ������� (3�r�b�g)
			uint Interleave[LAYER_COUNT_T];	// �C���^�[���[�u�� (3�r�b�g)
			uint Segment   [LAYER_COUNT_T];	// �Z�O�����g�� (4�r�b�g)
											// [�l�N�X�g���͎擾�ł��܂���]
			uint Phase;						// �A�����M�ʑ��␳�� (3�r�b�g)
			uint Reserved;					// ���U�[�u (12�r�b�g)
		};

		// [�@�\] ISDB-T �� TMCC �����擾
		virtual status GetTmccT(uint tuner, TmccT *tmcc) = 0;

		// [�@�\] ISDB-T ���b�N������擾
		// [����] ���C�������݂��A�Ȃ������̃��C�����G���[�t���[�ł���Ƃ��� true �ɂȂ�܂��B
		virtual status GetLockedT(uint tuner, bool locked[LAYER_COUNT_T]) = 0;

		// ��M�K�w
		enum LayerMask {
			LAYER_MASK_NONE,

			// ISDB-S
			LAYER_MASK_L = 1 << LAYER_INDEX_L,
			LAYER_MASK_H = 1 << LAYER_INDEX_H,

			// ISDB-T
			LAYER_MASK_A = 1 << LAYER_INDEX_A,
			LAYER_MASK_B = 1 << LAYER_INDEX_B,
			LAYER_MASK_C = 1 << LAYER_INDEX_C
		};

		// [�@�\] ��M�K�w�̐ݒ�
		// [����] ISDB-S �̒�K�w����M���Ȃ��悤�ɐݒ肷�邱�Ƃ͂ł��܂���B
		virtual status SetLayerEnable(uint tuner, ISDB isdb, LayerMask  layerMask)       = 0;
		virtual status GetLayerEnable(uint tuner, ISDB isdb, LayerMask *layerMask) const = 0;

		// --------
		// DMA �]��
		// --------

		// �o�b�t�@�T�C�Y
		enum {
			BUFFER_PAGE_COUNT = 511
		};

		// �o�b�t�@���
		struct BufferInfo {
			uint VirtualSize;
			uint VirtualCount;
			uint LockSize;
		};
		// �o�b�t�@�̓h���C�o������ VirtualAlloc(4096*BUFFER_PAGE_COUNT*VirtualSize) �� VirtualCount ��Ăяo�����Ƃɂ��m�ۂ���܂��B
		// VirtualCount �� 2 �ȏ�̏ꍇ�̓o�b�t�@����������邽�߁A�A�h���X���s�A���ɂȂ邱�Ƃɂ����ӂ��������B
		// LockSize �̓h���C�o�����Ń����������b�N����P�ʂł��B
		// 
		// VirtualSize �͈̔͂� 0 �ȊO�̔C�ӂ̐��l�ł��B
		// (VirtualSize * VirtualCount) �͓]���J�E���^�̃r�b�g���ɂ�鐧�����󂯂邽�߁A�͈͂� 1�`4095 �ł��B
		// (VirtualSize % LockSize) �� 0 �łȂ���΂Ȃ�܂���B
		// 
		// DMA �o�b�t�@�� CPU �����猩�ăL���b�V���֎~�ɂȂ��Ă��܂��B���̂��߁A�o�b�t�@�̓��e���o�C�g�P�ʂŕ�����
		// �ǂݏo���ꍇ�Ȃǂɑ��x�ቺ���������܂��B����������ɂ̓f�[�^���L���b�V���\�ȃ������ɃR�s�[���āA
		// �R�s�[���ꂽ�f�[�^�ɃA�N�Z�X���܂��B

		// [�@�\] DMA �o�b�t�@�̊m�ہE���
		// [����] DMA �o�b�t�@���J������ɂ� SetBufferInfo(NULL) �Ƃ��܂��B
		//        �o�b�t�@���m�ۂ���Ă��Ȃ��Ƃ��� GetBufferInfo() ���Ăяo���ƁAbufferInfo ���w���S�Ẵ����o�� 0 �ɂȂ�܂��B
		//        �o�b�t�@�̍\����ύX����ꍇ�́A���݂̃o�b�t�@��������Ă�����߂Ċm�ۂ��܂��B
		virtual status SetBufferInfo(const BufferInfo *bufferInfo)       = 0;
		virtual status GetBufferInfo(      BufferInfo *bufferInfo) const = 0;

		// [�@�\] DMA �o�b�t�@�̃|�C���^���擾
		// [����] index �Ŏw�肵�� DMA �o�b�t�@�̃|�C���^���擾���܂��Bindex �͈̔͂� 0 ���� BufferInfo::VirtualCount-1 �ł��B
		virtual status GetBufferPtr(uint index, void **ptr) const = 0;

		// [�@�\] �]���J�E���^�����Z�b�g�E�C���N�������g
		// [����] FPGA ��H�͎��̂悤�ɓ��삵�܂��B
		//			while (true) {
		//				/* �]���J�E���^���`�F�b�N */
		//				if (�]���J�E���^ == 0) {
		//					TransferInfo::TransferCounter0 = true;
		//					break;
		//				}
		//				if (�]���J�E���^ <= 1) {
		//					TransferInfo::TransferCounter1 = true;
		//					/* �����ł� break ���Ȃ� */
		//				}
		//
		//				/* �]���J�E���^���f�N�������g */
		//				�]���J�E���^--;
		//
		//				/* �f�[�^�]�� */
		//				for (uint i=0; i<BUFFER_PAGE_COUNT; i++) {
		//					/* 4096+64 �o�C�g�̃f�[�^�����܂�܂ő҂� */
		//					while (true) {
		//						if (4096+64 <= �o�b�t�@��̃f�[�^�o�C�g��) {
		//							break;
		//						}
		//					}
		//					/* 4096 �o�C�g�̃f�[�^��]�� */
		//					Transfer();
		//				}
		//			}
		//        �z�X�g������́A4096*BUFFER_PAGE_COUNT �o�C�g�P�ʂœ]���J�E���^���C���N�������g���邱�ƂɂȂ�܂��B
		//        �]���J�E���^���� 12 �r�b�g�ł��B
		virtual status ResetTransferCounter() = 0;
		virtual status IncrementTransferCounter() = 0;

		// [�@�\] �X�g���[�����Ƃ̓]������
		// [����] �e�X�g���[����]�����邩�ǂ�����ݒ肷�邱�Ƃ��ł��܂��B
		//        �K�v�̂Ȃ��X�g���[�����I�t�ɂ��邱�Ƃ� PCI �o�X�̑ш�𖳑ʂɎg�����Ƃ��Ȃ��Ȃ�܂��B
		//        DMA �]�����쒆�ɂ��ύX�\�ł��B
		virtual status SetStreamEnable(uint tuner, ISDB isdb, bool  enable)       = 0;
		virtual status GetStreamEnable(uint tuner, ISDB isdb, bool *enable) const = 0;

		// [�@�\] �X�g���[�����Ƃ� 3 �r�b�g�⏕�f�[�^�̐ݒ�
		// [����] 1 TS �p�P�b�g(188�o�C�g) �� 63 �}�C�N���p�P�b�g���g���ē]������܂����A
		//        3�o�C�g�~63�}�C�N���p�P�b�g=189�o�C�g�ƂȂ�A�����̃}�C�N���p�P�b�g�ɂ͖��g�p������ 1 �o�C�g����܂��B
		//        ���̃o�C�g�̉��� 3 �r�b�g�����[�U�[�����R�ɐݒ肷�邱�Ƃ��ł��܂��B
		//        ����IC ����̐M���� FPGA ���Ƀf�[�^����荞��ł���ł��邾������������ 3 �r�b�g�̃f�[�^��
		//        �t������܂��̂ŁA�^�C���X�^���v����ɗ��p���邱�Ƃ��ł��܂��B
		//        FPGA ���ł͒l�̏������݂� PCI �N���b�N�ɓ������A�l�̓ǂݏo���� TS �N���b�N�ɓ������Ă��܂��B
		//        ���̂��߁A�ݒ肷�鐔��̓O���C�R�[�h�Ȃǂ̃n�~���O������ 1 �̂��̂��g���Ă��������B
		virtual status SetStreamGray(uint tuner, ISDB isdb, uint  gray)       = 0;
		virtual status GetStreamGray(uint tuner, ISDB isdb, uint *gray) const = 0;

		// [�@�\] DMA �J�n�E��~�̐���
		// [����] DMA �]���͑S�� CPU ����݂��邱�ƂȂ����삵�܂��B
		//        GetTransferEnable() �� true  ��������Ƃ��� SetTransferEnable(true ) �Ƃ�����A
		//        GetTransferEnable() �� false ��������Ƃ��� SetTransferEnable(false) �Ƃ���ƃG���[�ɂȂ�܂��B
		//        
		//        GetTransferEnable() �Ŏ擾�ł���l�́A�P�� SetTransferEnable() �ōŌ�ɐݒ肳�ꂽ�l�Ɠ����ł��B
		//        �]���J�E���^�� 0 �ɂȂ�ȂǁA�n�[�h�E�F�A���� DMA �]���������I�ɒ�~����v��������������܂����A
		//        ���̏ꍇ�ł� GetTransferEnable() �œ�����l�͕ς��܂���B
		virtual status SetTransferEnable(bool  enable)       = 0;
		virtual status GetTransferEnable(bool *enable) const = 0;

		struct TransferInfo {
			bool TransferCounter0;	// �]���J�E���^�� 0 �ł���̂����o����
			bool TransferCounter1;	// �]���J�E���^�� 1 �ȉ��ł���̂����o����
			bool BufferOverflow;	// PCI �o�X�𒷊��ɓn��m�ۂł��Ȃ��������߁A�{�[�h��� FIFO(�T�C�Y=8MB) ����ꂽ
		};							// (�����̃t���O�́A��x�ł��������������o����� DMA �]�����ĊJ����܂ŃN���A����܂���)

		// [�@�\] DMA ��Ԃ̎擾
		virtual status GetTransferInfo(TransferInfo *) = 0;

		// �}�C�N���p�P�b�g�̍\��
		// +------------+----+----+----+----+----+----+----+----+----+----+----+
		// | �r�b�g�ʒu | 31 | 30 | 29 | 28 | 27 | 26 | 25 | 24 | 23 | .. |  0 |
		// +------------+----+----+----+----+----+----+----+----+----+----+----+
		// |    ���e    |      id      |    counter   | st | er |     data     |
		// +------------+--------------+--------------+----+----+--------------+
		// id     : �X�g���[��ID
		// counter: �X�g���[�����Ƃ̃J�E���^
		// st     : TS �p�P�b�g�J�n�ʒu�t���O
		// er     : �G���[�t���O (TransferCounter0 �� TransferCounter1 �� BufferOverflow �̘_���a)
		// data   : �f�[�^

		// �X�g���[��ID
		// +----+------------------------+
		// | id | ����                   |
		// +----+------------------------+
		// |  0 | �֎~                   |
		// |  1 | �`���[�i�[�ԍ�0 ISDB-S |
		// |  2 | �`���[�i�[�ԍ�0 ISDB-T |
		// |  3 | �`���[�i�[�ԍ�1 ISDB-S |
		// |  4 | �`���[�i�[�ԍ�1 ISDB-T |
		// |  5 | �\��                   |
		// |  6 | �\��                   |
		// |  7 | �\��                   |
		// +----+------------------------+
		// �X�g���[��ID �� 0 �ɂȂ邱�Ƃ͐�΂ɂ���܂���B
		// DMA �]�����ǂ��܂Ői��ł���̂��𒲂ׂ�ɂ́A�]���O�� �X�g���[��ID �� 0 �ɐݒ肵�āA
		// ���̉ӏ��� 0 �ȊO�ɂȂ������ǂ����𒲂ׂ܂��B
		// ���p��͓]���O�� 4 �o�C�g�̃}�C�N���p�P�b�g�̈�� 0 ���������݁A0 �ȊO�ɂȂ������ǂ����𒲂ׂ邱�ƂɂȂ�܂��B

		// �}�C�N���p�P�b�g���� TS �p�P�b�g���č\��������@�ɂ��Ă̓T���v���R�[�h�����Q�Ƃ��������B
		// ���̊֐����Ăяo��������� 188 �o�C�g�ɖ����Ȃ��p�P�b�g���������邱�Ƃ�����A�؎̂ď������K�v�ł��B
		// �ESetTunerSleep()
		// �ESetFrequency()
		// �ESetIdS()
		// �ESetLayerEnable()
		// �ESetStreamEnable()
		// �ESetTransferEnable(true)

	protected:
		virtual ~Device() {}
	};

	enum Status {
		// �G���[�Ȃ�
		STATUS_OK,

		// ��ʓI�ȃG���[
		STATUS_GENERAL_ERROR = (1)*256,
		STATUS_NOT_IMPLIMENTED,
		STATUS_INVALID_PARAM_ERROR,
		STATUS_OUT_OF_MEMORY_ERROR,
		STATUS_INTERNAL_ERROR,

		// �o�X�N���X�̃G���[
		STATUS_WDAPI_LOAD_ERROR = (2)*256,	// wdapi921.dll �����[�h�ł��Ȃ�
		STATUS_ALL_DEVICES_MUST_BE_DELETED_ERROR,

		// �f�o�C�X�N���X�̃G���[
		STATUS_PCI_BUS_ERROR = (3)*256,
		STATUS_CONFIG_REVISION_ERROR,
		STATUS_FPGA_VERSION_ERROR,
		STATUS_PCI_BASE_ADDRESS_ERROR,
		STATUS_FLASH_MEMORY_ERROR,

		STATUS_DCM_LOCK_TIMEOUT_ERROR,
		STATUS_DCM_SHIFT_TIMEOUT_ERROR,

		STATUS_I2C_ERROR,

		STATUS_PLL_OUT_OF_RANGE_ERROR,
		STATUS_PLL_LOCK_TIMEOUT_ERROR,

		STATUS_VIRTUAL_ALLOC_ERROR,
		STATUS_DMA_ADDRESS_ERROR,
		STATUS_BUFFER_ALREADY_ALLOCATED_ERROR,

		STATUS_DEVICE_IS_ALREADY_OPEN_ERROR,
		STATUS_DEVICE_IS_NOT_OPEN_ERROR,

		STATUS_BUFFER_IS_IN_USE_ERROR,
		STATUS_BUFFER_IS_NOT_ALLOCATED_ERROR,

		STATUS_DEVICE_MUST_BE_CLOSED_ERROR,

		// WinDriver �֘A�̃G���[
		STATUS_WD_DriverName_ERROR = (4)*256,

		STATUS_WD_Open_ERROR,
		STATUS_WD_Close_ERROR,

		STATUS_WD_License_ERROR,
		STATUS_WD_Version_ERROR,

		STATUS_WD_PciScanCards_ERROR,
		STATUS_WD_PciConfigDump_ERROR,
		STATUS_WD_PciGetCardInfo_ERROR,
		STATUS_WD_PciGetCardInfo_Bus_ERROR,
		STATUS_WD_PciGetCardInfo_Memory_ERROR,

		STATUS_WD_CardRegister_ERROR,
		STATUS_WD_CardUnregister_ERROR,

		STATUS_WD_CardCleanupSetup_ERROR,

		STATUS_WD_DMALock_ERROR,
		STATUS_WD_DMAUnlock_ERROR,

		STATUS_WD_DMASyncCpu_ERROR,
		STATUS_WD_DMASyncIo_ERROR
	};

	// ------------------------------
	// ����C�x���g�ɂ����铮��̏ڍ�
	// ------------------------------

	// 1. �{�[�h�̓d�����������ꂽ�Ƃ��́A�n�[�h�E�F�A�͈ȉ��̏�ԂɂȂ�܂��B
	//    �ESetLnbPower(LNB_POWER_OFF)
	//    �ESetTunerPowerReset(TUNER_POWER_OFF)
	//    �EDMA ����͒�~
	// 2. PCI ���Z�b�g���A�T�[�g���ꂽ�Ƃ����A�d���������Ɠ�����ԂɂȂ�܂��B
	// 3. �A�v���P�[�V�������ُ�I�������ꍇ���܂߃f�o�C�X���N���[�Y����Ƃ��́A�ȉ��̏��������ԂɎ��s����܂��B
	//    �EDMA ������~
	//    �ESetLnbPower([SetLnbPowerWhenClose() �Ŏw�肳�ꂽ�l])
	//    �ESetTunerPowerReset(TUNER_POWER_OFF)
	//    �EDMA �o�b�t�@�����
}
}

#endif
