#pragma once

#include "stdafx.h"
#include "asio.h"
#include "asiodrivers.h"
#include "WaveFile.h"

LRESULT CALLBACK SoundOutProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow);
ATOM MyRegisterClass(HINSTANCE hInstance);

void OnInitDialog(HWND hDlg);
bool OnSelectFile(HWND hDlg);
void StopSoundOutput(HWND hDlg);
bool PlaySoundOutput(HWND hDlg);
void PauseSoundOutput(HWND hDlg);
bool OpenSoundOutDevice(HWND hDlg, UINT num, WAVEFORMATEX* wf);
void bufferSwitch(long index, ASIOBool processNow);
void CloseSoundOutputDevice();

HWND g_hWnd = nullptr;					// �A�v���P�[�V�����̃E�C���h�E�n���h��.
HINSTANCE g_hInstance = nullptr;
WCHAR szTitle[100];                  // �^�C�g�� �o�[�̃e�L�X�g
WCHAR szWindowClass[100];            // ���C�� �E�B���h�E �N���X��

AsioDrivers* g_asioDrivers = nullptr;		// ASIO�h���C�o�̃I�u�W�F�N�g.
ASIOBufferInfo	BufInfo[16];				//!< �e�`�����l���̃o�b�t�@���e�[�u��.
BYTE*			DataBuff;					//!< �f�[�^�o�b�t�@.
DWORD			DataSize;					//!< �f�[�^�T�C�Y.
DWORD			DevCnt;						//!< ASIO�f�o�C�X�̌�.
char				DevName[10][32];			//!< ASIO�f�o�C�X�̖��O�e�[�u��.
TCHAR			FileName[MAX_PATH + 1];		//!< �Đ�����WAV�t�@�C���̃t�@�C����.
DWORD			BufSize;					//!< ASIO�o�b�t�@�̃o�C�g��.
BYTE*			WavBuf[2];					//!< WAV�t�@�C���o�b�t�@�𕡐��u���b�N�ɕ��������|�C���^�e�[�u��.
DWORD			WavSize[2];					//!< WAV�t�@�C���o�b�t�@�̕����u���b�N�ɓǂݍ��񂾃f�[�^�̃o�C�g��.
int				ActivePage;					//!< ASIO�h���C�o���Đ����Ă��镪���u���b�N�̔ԍ�.
DWORD			ReadCnt;					//!< ASIO�h���C�o���Đ����Ă��镪���u���b�N�̓ǂݍ��݃J�E���^.

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// ���[�J���ϐ�������������.
	g_hWnd = NULL;
	DataBuff = NULL;
	DataSize = 0;
	DevCnt = 0;
	memset(DevName, 0, sizeof(DevName));
	memset(FileName, 0, sizeof(FileName));
		
	// �O���[�o������������������Ă��܂��B
	LoadStringW(hInstance, 103, szTitle, 100);
	LoadStringW(hInstance, 109, szWindowClass, 100);
	MyRegisterClass(hInstance);

	// �A�v���P�[�V�����̏����������s���܂�:
	InitInstance(hInstance, nCmdShow);

	//! �R�����R���g���[��������������.
	/*
	INITCOMMONCONTROLSEX ic;
	ic.dwSize = sizeof(INITCOMMONCONTROLSEX);
	ic.dwICC = ICC_COOL_CLASSES;
	InitCommonControlsEx(&ic);
	//! �_�C�A���O���I�[�v������.
	DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_MAIN), NULL, (DLGPROC)SoundOutProc);
	*/

 	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(109));

	MSG msg;
	// ���C�� ���b�Z�[�W ���[�v:
	do {
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			//----------------------------------------------------------
			// �A�b�v�f�[�g
			try
			{
				// if (CInput::GetInstance().GetKeyDown(KEY_L)) ride->Play();

			}
			catch (...)
			{
				break;
			}
		}
	} while (msg.message != WM_QUIT);

	return (int)msg.wParam;
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	g_hInstance = hInstance; // �O���[�o���ϐ��ɃC���X�^���X�������i�[���܂��B

	g_hWnd = CreateWindowW(szWindowClass, szTitle, WS_POPUP || WS_SIZEBOX,
		NULL, NULL, 1280, 720, nullptr, nullptr, hInstance, nullptr);

	if (!g_hWnd)
	{
		return FALSE;
	}

	ShowWindow(g_hWnd, nCmdShow);
	UpdateWindow(g_hWnd);

	return TRUE;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = SoundOutProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = 0;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = NULL;

	ShowCursor(FALSE);
	return RegisterClassExW(&wcex);
}

/*!
*	@brief	�_�C�A���O�̃��b�Z�[�W�E�n���h��.
*
*	@param	[in]	hDlg	�_�C�A���O�{�b�N�X�̃n���h��.
*	@param	[in]	msg		���b�Z�[�W.
*	@param	[in]	wParam	�ŏ��̃��b�Z�[�W�p�����[�^.
*	@param	[in]	lParam	2�Ԗڂ̃��b�Z�[�W�p�����[�^.
*
*	@retval	TRUE  = ���b�Z�[�W����������.
*	@retval	FALSE = ���b�Z�[�W���������Ȃ�����.
*/
LRESULT CALLBACK SoundOutProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		OnInitDialog(hDlg);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_FILE_SELECT:
			OnSelectFile(hDlg);
			break;

		case IDC_PLAY:
			PlaySoundOutput(hDlg);
			break;

		case IDC_PAUSE:
			PauseSoundOutput(hDlg);
			break;

		case IDC_STOP:
			StopSoundOutput(hDlg);
			break;

		case IDCANCEL:
			StopSoundOutput(hDlg);
			EndDialog(hDlg, 0);
			break;

		default:
			return FALSE;
		}
		break;

	case WM_USER:
		WavSize[wParam] = ReadWaveFile(WavBuf[wParam], DataSize >> 1);
		break;

	default:
		break;
	}
	return FALSE;
}

/****************************************************************************/
/*!
*	@brief	�_�C�A���O�̏�������.
*
*	@param	[in]	hDlg	�_�C�A���O�̃E�C���h�E�E�n���h��.
*
*	@retval	�Ȃ�.
*/
void OnInitDialog(HWND hDlg)
{
	//! ASIO�h���C�o�̃I�u�W�F�N�g���쐬����.
	if (!g_asioDrivers) {
		g_asioDrivers = new AsioDrivers();
	}
	//! �f�o�C�X�����i�[����z����쐬����.
	char* dev_list[10];
	for (DWORD i = 0; i < 10; i++) {
		dev_list[i] = DevName[i];
	}
	//! ASIO�f�o�C�X�����X�g���擾����.
	DevCnt = g_asioDrivers->getDriverNames(dev_list, 10);

	//! ASIO�f�o�C�X�����X�g���쐬����.
	HWND hItem = GetDlgItem(hDlg, IDC_OUTPUT_DEVICE);

	ComboBox_ResetContent(hItem);

	for (DWORD i = 0; i < DevCnt; i++) {
		ComboBox_AddString(hItem, DevName[i]);
	}
	ComboBox_SetCurSel(hItem, 0);
}

/****************************************************************************/
/*!
*	@brief	�Đ�����WAV�t�@�C����I������.
*
*	@param	[in]	hDlg	�_�C�A���O�̃E�C���h�E�E�n���h��.
*
*	@retval	�Ȃ�.
*/
bool OnSelectFile(HWND hDlg)
{
	//! �T�E���h�o�͂���t�@�C�����t�@�C���I���_�C�A���O����I��.
	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));
	memset(&FileName, 0, sizeof(FileName));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hDlg;
	ofn.lpstrFilter = _T("WAVE�t�@�C��(*.wav)\0*.wav\0�S�Ẵt�@�C��(*.*)\0*.*\0\0");
	ofn.lpstrFile = FileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = NULL;
	ofn.lpstrTitle = _T("�Đ��t�@�C��");
	if (GetOpenFileName(&ofn)) {
		SetWindowText(GetDlgItem(hDlg, IDC_FILENAME), FileName);
		return TRUE;
	}
	return FALSE;
}

/*!
*	@brief	WAV�t�@�C�����Đ�����.
*
*	@param	[in]	hDlg	�_�C�A���O�̃E�C���h�E�E�n���h��.
*
*	@retval	TRUE = OK. / FALSE = NG.
*/
bool PlaySoundOutput(HWND hDlg)
{
	//! WAV�t�@�C�����I�[�v�����Ă��Ȃ��ꍇ�A�I�[�v������.
	if (!IsOpenWaveFile()) {

		//! -1.�_�C�A���O����Đ�����WAV�t�@�C�������擾����.
		GetWindowText(GetDlgItem(hDlg, IDC_FILENAME), FileName, sizeof(FileName) - 1);

		//! -2.�Đ�����WAV�t�@�C�������I�[�v������.
		if (!ReadOpenWaveFile(FileName)) {
			MessageBox(hDlg, _T("�t�@�C�����I�[�v���ł��܂���"), _T("WAV�t�@�C���̍Đ�"), MB_OK);
			return FALSE;
		}
	}
	if (!g_hWnd) {
		//! �I�[�v������WAV�t�@�C���̃I�[�f�B�I�E�t�B�[�}�b�g���擾����.
		WAVEFORMATEX wf;
		GetAudioFormat(&wf);

		//! �T�E���h�o�̓f�o�C�X���I�[�v������.
		UINT num = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_OUTPUT_DEVICE));
		if (OpenSoundOutDevice(hDlg, num, &wf)) {
			//! WAV�t�@�C���̓ǂݍ��ݗp�o�b�t�@���m�ۂ���.
			if (!DataBuff) {
				DataSize = BufSize * sizeof(WORD) * 32;
				DataBuff = new BYTE[DataSize];
			}
			//! WAVE�t�@�C����ǂݍ���.
			WavBuf[0] = DataBuff;
			WavBuf[1] = &DataBuff[DataSize >> 1];
			WavSize[0] = ReadWaveFile(WavBuf[0], DataSize >> 1);
			WavSize[1] = ReadWaveFile(WavBuf[1], DataSize >> 1);
			ActivePage = 0;
			ReadCnt = 0;
		}
		g_hWnd = hDlg;
	}
	//! �T�E���h�o�͂��J�n����.
	ASIOStart();

	return TRUE;
}

/****************************************************************************/
/*!
*	@brief	WAV�t�@�C���̍Đ����ꎞ��~����.
*
*	@param	[in]	hDlg	�_�C�A���O�̃E�C���h�E�E�n���h��.
*
*	@retval	�Ȃ�.
*/
void PauseSoundOutput(HWND hDlg)
{
	//! �I�[�f�B�I���o�͂��~����.
	ASIOStop();

	EnableWindow(GetDlgItem(hDlg, IDC_PLAY), TRUE);
	EnableWindow(GetDlgItem(hDlg, IDC_PAUSE), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_STOP), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_FILENAME), TRUE);
	EnableWindow(GetDlgItem(hDlg, IDC_FILE_SELECT), TRUE);
	EnableWindow(GetDlgItem(hDlg, IDC_OUTPUT_DEVICE), TRUE);
}


/****************************************************************************/
/*!
*	@brief	WAV�t�@�C���̍Đ����~����.
*
*	@param	�Ȃ�.
*
*	@retval	�Ȃ�.
*/
void StopSoundOutput(HWND hDlg)
{
	//! �T�E���h�o�̓f�o�C�X���N���[�Y����.
	CloseSoundOutputDevice();

	//! �Đ�����WAV�t�@�C�����N���[�Y����.
	CloseWaveFile();

	//! WAV�t�@�C���̓ǂݍ��ݗp�o�b�t�@���������.
	if (DataBuff) {
		delete[] DataBuff;
		DataBuff = NULL;
	}
	g_hWnd = NULL;
	EnableWindow(GetDlgItem(hDlg, IDC_PLAY), TRUE);
	EnableWindow(GetDlgItem(hDlg, IDC_PAUSE), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_STOP), FALSE);
	EnableWindow(GetDlgItem(hDlg, IDC_FILENAME), TRUE);
	EnableWindow(GetDlgItem(hDlg, IDC_FILE_SELECT), TRUE);
	EnableWindow(GetDlgItem(hDlg, IDC_OUTPUT_DEVICE), TRUE);
}

/****************************************************************************/
/*!
*	@brief	ASIO�h���C�o���I�[�v������.
*
*	@param	[in]	hDlg	�_�C�A���O�̃E�C���h�E�E�n���h��.
*	@param	[in]	num		�I�[�v������f�o�C�X�ԍ�.
*	@param	[in]	wf		�Đ�����WAV�t�@�C���̃I�[�f�B�I���\���̂̃|�C���^.
*
*	@retval	�Ȃ�.
*/
bool OpenSoundOutDevice(HWND hDlg, UINT num, WAVEFORMATEX* wf)
{
	if (num >= DevCnt) {
		return FALSE;
	}
	//! �I�[�v������h���C�o�����[�h����.
	g_asioDrivers->loadDriver(DevName[num]);

	do {
		//! ASIO�f�o�C�X���I�[�v������.
		ASIODriverInfo info;
		memset(&info, 0, sizeof(info));
		info.asioVersion = 0;
		info.sysRef = hDlg;
		if (ASIOInit(&info) != ASE_OK) {
			break;
		}
		//! �쐬�\�ȃo�b�t�@�T�C�Y���擾����.
		long minimum;
		long maximum;
		long preferred;
		long granularity;
		if (ASIOGetBufferSize(&minimum, &maximum, &preferred, &granularity) != ASE_OK) {
			break;
		}
		//! �o�b�t�@�����i�[����̈������������.
		long i = 0;
		memset(BufInfo, 0, sizeof(BufInfo));
		for (; i < 2; i++) {
			BufInfo[i].isInput = ASIOFalse;
			BufInfo[i].channelNum = i;
		}
		BufSize = preferred;

		//! �g�p����`�����l���̃f�[�^�o�b�t�@���m�ۂ���.
		static ASIOCallbacks cb;
		cb.bufferSwitch = &bufferSwitch;
		cb.sampleRateDidChange = NULL;
		cb.asioMessage = NULL;
		cb.bufferSwitchTimeInfo = NULL;
		ASIOCreateBuffers(BufInfo, i, BufSize, &cb);

		//! �T���v�����[�g��ݒ肷��.
		ASIOSetSampleRate(wf->nSamplesPerSec);
		return TRUE;

	} while (FALSE);

	CloseSoundOutputDevice();
	return FALSE;
}

/****************************************************************************/
/*!
*	@brief	ASIO�h���C�o���N���[�Y����.
*
*	@param	�Ȃ�.
*
*	@retval	�Ȃ�.
*/
void CloseSoundOutputDevice()
{
	//! �I�[�f�B�I���o�͂��~����.
	ASIOStop();

	//! ���o�̓o�b�t�@���������.
	ASIODisposeBuffers();

	//! ASIO���N���[�Y����.
	ASIOExit();

	//! �h���C�o���A�����[�h����.
	if (g_asioDrivers) {
		g_asioDrivers->removeCurrentDriver();
	}
}

/****************************************************************************/
/*!
*	@brief	�f�[�^�o�b�t�@�̐؂�ւ��ʒm.
*
*	@param	[in]	index		�o�b�t�@�ԍ�.
*	@param	[in]	processNow	ASIOTrue = �����ɏ������\.
*
*	@retval	�Ȃ�.
*/
void bufferSwitch(long index, ASIOBool processNow)
{
	if (WavSize[0] > 0 || WavSize[1] > 0) {
		if (ReadCnt >= WavSize[ActivePage]) {
			PostMessage(g_hWnd, WM_USER, ActivePage, 0);

			ActivePage = (ActivePage + 1) & 1;
			ReadCnt = 0;
		}
	}
	BYTE*	buf_l = (BYTE*)(BufInfo[0].buffers[index]);
	BYTE*	buf_r = (BYTE*)(BufInfo[1].buffers[index]);

	//! �`�����l�������擾����.
	ASIOChannelInfo info;
	info.channel = BufInfo[0].channelNum;
	info.isInput = BufInfo[0].isInput;
	ASIOGetChannelInfo(&info);

	switch (info.type) {
	case ASIOSTInt16LSB:
		//! 16�r�b�g�I�[�f�B�I�̏ꍇ.
		memset(buf_l, 0, BufSize * 2);
		memset(buf_r, 0, BufSize * 2);
		if (WavSize[ActivePage] > 0) {
			short*	dst_l = (short*)buf_l;
			short*	dst_r = (short*)buf_r;
			short*	src = (short*)&(WavBuf[ActivePage][ReadCnt]);
			DWORD	limit = (WavSize[ActivePage] - ReadCnt) / 4;
			if (limit > BufSize) {
				limit = BufSize;
			}
			for (DWORD i = 0; i < limit; i++) {
				*(dst_l++) = *(src++);
				*(dst_r++) = *(src++);
			}
			ReadCnt += (BufSize * 4);
		}
		break;

	case ASIOSTInt24LSB:
		//! 24�r�b�g�I�[�f�B�I�̏ꍇ.
		memset(buf_l, 0, BufSize * 3);
		memset(buf_r, 0, BufSize * 3);
		if (WavSize[ActivePage] > 0) {
			BYTE*	dst_l = buf_l;
			BYTE*	dst_r = buf_r;
			BYTE*	src = &(WavBuf[ActivePage][ReadCnt]);
			DWORD	limit = (WavSize[ActivePage] - ReadCnt) / 4;
			if (limit > BufSize) {
				limit = BufSize;
			}
			for (DWORD i = 0; i < limit; i++) {
				*(dst_l++) = 0x00;
				*(dst_l++) = *(src++);
				*(dst_l++) = *(src++);

				*(dst_r++) = 0x00;
				*(dst_r++) = *(src++);
				*(dst_r++) = *(src++);
			}
			ReadCnt += (BufSize * 4);
		}
		break;

	case ASIOSTInt32LSB:
		//! 32�r�b�g�I�[�f�B�I�̏ꍇ.
		memset(buf_l, 0, BufSize * 4);
		memset(buf_r, 0, BufSize * 4);
		if (WavSize[ActivePage] > 0) {
			short*	dst_l = (short*)buf_l;
			short*	dst_r = (short*)buf_r;
			short*	src = (short*)&(WavBuf[ActivePage][ReadCnt]);
			DWORD	limit = (WavSize[ActivePage] - ReadCnt) / 4;
			if (limit > BufSize) {
				limit = BufSize;
			}
			for (DWORD i = 0; i < limit; i++) {
				*(dst_l++) = 0;
				*(dst_l++) = *(src++);
				*(dst_r++) = 0;
				*(dst_r++) = *(src++);
			}
			ReadCnt += (BufSize * 4);
		}
		break;

	case ASIOSTInt32LSB16:
	case ASIOSTInt32LSB18:
	case ASIOSTInt32LSB20:
	case ASIOSTInt32LSB24:
	case ASIOSTFloat32LSB:
	case ASIOSTInt32MSB:
	case ASIOSTInt32MSB16:
	case ASIOSTInt32MSB18:
	case ASIOSTInt32MSB20:
	case ASIOSTInt32MSB24:
	case ASIOSTFloat32MSB:
		memset(buf_l, 0, BufSize * 4);
		memset(buf_r, 0, BufSize * 4);
		break;

	case ASIOSTFloat64LSB:
	case ASIOSTFloat64MSB:
		memset(buf_l, 0, BufSize * 8);
		memset(buf_r, 0, BufSize * 8);
		break;

	case ASIOSTInt16MSB:
		memset(buf_l, 0, BufSize * 2);
		memset(buf_r, 0, BufSize * 2);
		break;

	case ASIOSTInt24MSB:
		memset(buf_l, 0, BufSize * 3);
		memset(buf_r, 0, BufSize * 3);
		break;
	}
	// ASIOOutputReady()���T�|�[�g���Ă���ꍇ�A����ASIOOutputReady()���Ăяo��.
	ASIOOutputReady();
}