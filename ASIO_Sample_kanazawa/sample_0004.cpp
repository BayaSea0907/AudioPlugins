/****************************************************************************
 *	Copylight(C) 2011 Kanazawa-soft-design,LLC.All Rights Reserved.
 ****************************************************************************/
/*!
 *	@file	sample_0004.cpp
 *
 *	@brief	ASIOを利用したサウンド出力プログラム.
 *
 *	@author	金澤 宣明
 */
#include <windows.h>
#include <windowsX.h>
#include <stdio.h>
#include <commctrl.h>
#include <tchar.h>
#include <math.h>
#include "resource.h"
#include "WaveFile.h"
#include "ginclude.h"
// #include "asio.h"
// #include "asiodrivers.h"


// 関数プロトタイプ.
static LRESULT CALLBACK	SoundOutProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
static void				OnInitDialog( HWND hDlg );
static BOOL				OnSelectFile( HWND hDlg );
static BOOL				PlaySoundOutput( HWND hDlg );
static void				PauseSoundOutput( HWND hDlg );
static void				StopSoundOutput( HWND hDlg );
static BOOL				OpenSoundOutDevice( HWND hDlg, UINT num, WAVEFORMATEX* wf );
static void				CloseSoundOutputDevice( void );
static void				bufferSwitch( long index, ASIOBool processNow );

// 外部変数.
extern AsioDrivers*		asioDrivers;				//!< ASIOドライバのオブジェクト.

// ローカル変数.
static HWND				AppWnd;						//!< アプリケーションのウインドウハンドル.
static ASIOBufferInfo	BufInfo[16];				//!< 各チャンネルのバッファ情報テーブル.
static BYTE*			DataBuff;					//!< データバッファ.
static DWORD			DataSize;					//!< データサイズ.
static DWORD			DevCnt;						//!< ASIOデバイスの個数.
static char				DevName[10][32];			//!< ASIOデバイスの名前テーブル.
static TCHAR			FileName[MAX_PATH+1];		//!< 再生するWAVファイルのファイル名.
static DWORD			BufSize;					//!< ASIOバッファのバイト数.
static BYTE*			WavBuf[2];					//!< WAVファイルバッファを複数ブロックに分割したポインタテーブル.
static DWORD			WavSize[2];					//!< WAVファイルバッファの分割ブロックに読み込んだデータのバイト数.
static int				ActivePage;					//!< ASIOドライバが再生している分割ブロックの番号.
static DWORD			ReadCnt;					//!< ASIOドライバが再生している分割ブロックの読み込みカウンタ.


/****************************************************************************/
/*!
 *	@brief	アプリケーション・メイン.
 *
 *	@param	[in]	hInst		現在のインスタンスのハンドル.
 *	@param	[in]	hPreInst	以前のインスタンスのハンドル.
 *	@param	[in]	CmdLine		コマンドライン.
 *	@param	[in]	show		表示状態.
 *
 *	@retval	常に0を返す.
 */
int WINAPI	WinMain( HINSTANCE hInst, HINSTANCE hPreInst, LPSTR CmdLine, int show )
{
	//! ローカル変数を初期化する.
	AppWnd   = NULL;
	DataBuff = NULL;
	DataSize = 0;
	DevCnt   = 0;
	memset( DevName , 0, sizeof(DevName)  );
	memset( FileName, 0, sizeof(FileName) );

	//! コモンコントロールを初期化する.
    INITCOMMONCONTROLSEX ic;
	ic.dwSize = sizeof(INITCOMMONCONTROLSEX);
	ic.dwICC  = ICC_COOL_CLASSES;
	InitCommonControlsEx( &ic );

	//! ダイアログをオープンする.
	DialogBox( hInst, MAKEINTRESOURCE(IDD_DIALOG_MAIN), NULL, (DLGPROC)SoundOutProc );

	return 0;
}

/****************************************************************************/
/*!
 *	@brief	ダイアログのメッセージ・ハンドラ.
 *
 *	@param	[in]	hDlg	ダイアログボックスのハンドル.
 *	@param	[in]	msg		メッセージ.
 *	@param	[in]	wParam	最初のメッセージパラメータ.
 *	@param	[in]	lParam	2番目のメッセージパラメータ.
 *
 *	@retval	TRUE  = メッセージを処理した.
 *	@retval	FALSE = メッセージを処理しなかった.
 */
static LRESULT CALLBACK		SoundOutProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg ){
	case WM_INITDIALOG:
		OnInitDialog( hDlg );
		break;

	case WM_COMMAND:
		switch( LOWORD(wParam) ){
		case IDC_FILE_SELECT:
			OnSelectFile( hDlg );
			break;

		case IDC_PLAY:
			PlaySoundOutput( hDlg );
			break;

		case IDC_PAUSE:
			PauseSoundOutput( hDlg );
			break;

		case IDC_STOP:
			StopSoundOutput( hDlg );
			break;

		case IDCANCEL:
			StopSoundOutput( hDlg );
			EndDialog( hDlg, 0 );
			break;

		default:
			return FALSE;
		}
		break;

	case WM_USER:
		WavSize[wParam] = ReadWaveFile( WavBuf[wParam], DataSize >> 1 );
		break;

	default:
		break;
	}
	return FALSE;
}

/****************************************************************************/
/*!
 *	@brief	ダイアログの初期処理.
 *
 *	@param	[in]	hDlg	ダイアログのウインドウ・ハンドル.
 *
 *	@retval	なし.
 */
static void		OnInitDialog( HWND hDlg )
{
	//! ASIOドライバのオブジェクトを作成する.
	if( !asioDrivers ){
		asioDrivers = new AsioDrivers();
	}
	//! デバイス名を格納する配列を作成する.
	char* dev_list[10];
	for( DWORD i = 0; i < 10; i++ ){
		dev_list[i] = DevName[i];
	}
	//! ASIOデバイス名リストを取得する.
	DevCnt = asioDrivers->getDriverNames( dev_list, 10 );

	//! ASIOデバイス名リストを作成する.
	HWND hItem = GetDlgItem( hDlg, IDC_OUTPUT_DEVICE );

	ComboBox_ResetContent( hItem );

	for( DWORD i = 0; i < DevCnt; i++ ){
		ComboBox_AddString( hItem, DevName[i] );
	}
	ComboBox_SetCurSel( hItem, 0 );
}

/****************************************************************************/
/*!
 *	@brief	再生するWAVファイルを選択する.
 *
 *	@param	[in]	hDlg	ダイアログのウインドウ・ハンドル.
 *
 *	@retval	なし.
 */
static BOOL		OnSelectFile( HWND hDlg )
{
	//! サウンド出力するファイルをファイル選択ダイアログから選ぶ.
	OPENFILENAME ofn;
	memset( &ofn     , 0, sizeof(ofn)      );
	memset( &FileName, 0, sizeof(FileName) );
	ofn.lStructSize    = sizeof(ofn);
	ofn.hwndOwner      = hDlg;
	ofn.lpstrFilter    = _T("WAVEファイル(*.wav)\0*.wav\0全てのファイル(*.*)\0*.*\0\0");
	ofn.lpstrFile      = FileName;
	ofn.nMaxFile       = MAX_PATH;
	ofn.Flags          = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt    = NULL;
	ofn.lpstrTitle     = _T("再生ファイル");
	if( GetOpenFileName( &ofn ) ){
		SetWindowText( GetDlgItem( hDlg, IDC_FILENAME ), FileName );
		return TRUE;
	}
	return FALSE;
}

/****************************************************************************/
/*!
 *	@brief	WAVファイルを再生する.
 *
 *	@param	[in]	hDlg	ダイアログのウインドウ・ハンドル.
 *
 *	@retval	TRUE = OK. / FALSE = NG.
 */
static BOOL		PlaySoundOutput( HWND hDlg )
{
	//! WAVファイルをオープンしていない場合、オープンする.
	if( !IsOpenWaveFile() ){

		//! -1.ダイアログから再生するWAVファイル名を取得する.
		GetWindowText( GetDlgItem( hDlg, IDC_FILENAME ), FileName, sizeof(FileName) - 1 );

		//! -2.再生するWAVファイル名をオープンする.
		if( !ReadOpenWaveFile( FileName ) ){
			MessageBox( hDlg, _T("ファイルをオープンできません"), _T("WAVファイルの再生"), MB_OK );
			return FALSE;
		}
	}
	if( !AppWnd ){
		//! オープンしたWAVファイルのオーディオ・フィーマットを取得する.
		WAVEFORMATEX wf;
		GetAudioFormat( &wf );

		//! サウンド出力デバイスをオープンする.
		UINT num = ComboBox_GetCurSel( GetDlgItem( hDlg, IDC_OUTPUT_DEVICE ) );
		if( OpenSoundOutDevice( hDlg, num, &wf ) ){
			//! WAVファイルの読み込み用バッファを確保する.
			if( !DataBuff ){
				DataSize = BufSize * sizeof(WORD) * 32;
				DataBuff = new BYTE[DataSize];
			}
			//! WAVEファイルを読み込む.
			WavBuf[0]  = DataBuff;
			WavBuf[1]  = &DataBuff[DataSize >> 1];
			WavSize[0] = ReadWaveFile( WavBuf[0], DataSize >> 1 );
			WavSize[1] = ReadWaveFile( WavBuf[1], DataSize >> 1 );
			ActivePage = 0;
			ReadCnt    = 0;
		}
		AppWnd = hDlg;
	}
	//! サウンド出力を開始する.
	ASIOStart();

	EnableWindow( GetDlgItem( hDlg, IDC_PLAY          ), FALSE );
	EnableWindow( GetDlgItem( hDlg, IDC_PAUSE         ), TRUE  );
	EnableWindow( GetDlgItem( hDlg, IDC_STOP          ), TRUE  );
	EnableWindow( GetDlgItem( hDlg, IDC_FILENAME      ), FALSE );
	EnableWindow( GetDlgItem( hDlg, IDC_FILE_SELECT   ), FALSE );
	EnableWindow( GetDlgItem( hDlg, IDC_OUTPUT_DEVICE ), FALSE );
	return TRUE;
}

/****************************************************************************/
/*!
 *	@brief	WAVファイルの再生を一時停止する.
 *
 *	@param	[in]	hDlg	ダイアログのウインドウ・ハンドル.
 *
 *	@retval	なし.
 */
static void		PauseSoundOutput( HWND hDlg )
{
	//! オーディオ入出力を停止する.
	ASIOStop();

	EnableWindow( GetDlgItem( hDlg, IDC_PLAY          ), TRUE  );
	EnableWindow( GetDlgItem( hDlg, IDC_PAUSE         ), FALSE );
	EnableWindow( GetDlgItem( hDlg, IDC_STOP          ), FALSE );
	EnableWindow( GetDlgItem( hDlg, IDC_FILENAME      ), TRUE  );
	EnableWindow( GetDlgItem( hDlg, IDC_FILE_SELECT   ), TRUE  );
	EnableWindow( GetDlgItem( hDlg, IDC_OUTPUT_DEVICE ), TRUE  );
}

/****************************************************************************/
/*!
 *	@brief	WAVファイルの再生を停止する.
 *
 *	@param	なし.
 *
 *	@retval	なし.
 */
static void		StopSoundOutput( HWND hDlg )
{
	//! サウンド出力デバイスをクローズする.
	CloseSoundOutputDevice();

	//! 再生したWAVファイルをクローズする.
	CloseWaveFile();

	//! WAVファイルの読み込み用バッファを解放する.
	if( DataBuff ){
		delete [] DataBuff;
		DataBuff = NULL;
	}
	AppWnd = NULL;
	EnableWindow( GetDlgItem( hDlg, IDC_PLAY          ), TRUE  );
	EnableWindow( GetDlgItem( hDlg, IDC_PAUSE         ), FALSE );
	EnableWindow( GetDlgItem( hDlg, IDC_STOP          ), FALSE );
	EnableWindow( GetDlgItem( hDlg, IDC_FILENAME      ), TRUE  );
	EnableWindow( GetDlgItem( hDlg, IDC_FILE_SELECT   ), TRUE  );
	EnableWindow( GetDlgItem( hDlg, IDC_OUTPUT_DEVICE ), TRUE  );
}

/****************************************************************************/
/*!
 *	@brief	ASIOドライバをオープンする.
 *
 *	@param	[in]	hDlg	ダイアログのウインドウ・ハンドル.
 *	@param	[in]	num		オープンするデバイス番号.
 *	@param	[in]	wf		再生するWAVファイルのオーディオ情報構造体のポインタ.
 *
 *	@retval	なし.
 */
static BOOL		OpenSoundOutDevice( HWND hDlg, UINT num, WAVEFORMATEX* wf )
{
	if( num >= DevCnt ){
		return FALSE;
	}
	//! オープンするドライバをロードする.
	asioDrivers->loadDriver( DevName[num] );

	do {
		//! ASIOデバイスをオープンする.
		ASIODriverInfo info;
		memset( &info, 0, sizeof(info) );
		info.asioVersion = 0;
		info.sysRef      = hDlg;
		if( ASIOInit( &info ) != ASE_OK ){
			break;
		}
		//! 作成可能なバッファサイズを取得する.
		long minimum;
		long maximum;
		long preferred;
		long granularity;
		if( ASIOGetBufferSize( &minimum, &maximum, &preferred, &granularity ) != ASE_OK ){
			break;
		}
		//! バッファ情報を格納する領域を初期化する.
		long i = 0;
		memset( BufInfo, 0, sizeof(BufInfo) );
		for( ; i < 2; i++ ){
			BufInfo[i].isInput    = ASIOFalse;
			BufInfo[i].channelNum = i;
		}
		BufSize = preferred;

		//! 使用するチャンネルのデータバッファを確保する.
		static ASIOCallbacks cb;
		cb.bufferSwitch         = &bufferSwitch;
		cb.sampleRateDidChange  = NULL;
		cb.asioMessage          = NULL;
		cb.bufferSwitchTimeInfo = NULL;
		ASIOCreateBuffers( BufInfo, i, BufSize, &cb );

		//! サンプルレートを設定する.
		ASIOSetSampleRate( wf->nSamplesPerSec );
		return TRUE;

	}while( FALSE );

	CloseSoundOutputDevice();
	return FALSE;
}

/****************************************************************************/
/*!
 *	@brief	ASIOドライバをクローズする.
 *
 *	@param	なし.
 *
 *	@retval	なし.
 */
static void		CloseSoundOutputDevice( void )
{
	//! オーディオ入出力を停止する.
	ASIOStop();

	//! 入出力バッファを解放する.
	ASIODisposeBuffers();

	//! ASIOをクローズする.
	ASIOExit();

	//! ドライバをアンロードする.
	if( asioDrivers ){
		asioDrivers->removeCurrentDriver();
	}
}

/****************************************************************************/
/*!
 *	@brief	データバッファの切り替え通知.
 *
 *	@param	[in]	index		バッファ番号.
 *	@param	[in]	processNow	ASIOTrue = すぐに処理が可能.
 *
 *	@retval	なし.
 */
static void		bufferSwitch( long index, ASIOBool processNow )
{
	if( WavSize[0] > 0 || WavSize[1] > 0 ){
		if( ReadCnt >= WavSize[ActivePage] ){
			PostMessage( AppWnd, WM_USER, ActivePage, 0 );

			ActivePage = (ActivePage + 1) & 1;
			ReadCnt    = 0;
		}
	}
	BYTE*	buf_l = (BYTE*)(BufInfo[0].buffers[index]);
	BYTE*	buf_r = (BYTE*)(BufInfo[1].buffers[index]);

	//! チャンネル情報を取得する.
	ASIOChannelInfo info;
	info.channel = BufInfo[0].channelNum;
	info.isInput = BufInfo[0].isInput;
	ASIOGetChannelInfo( &info );

	switch( info.type ){
	case ASIOSTInt16LSB:
		//! 16ビットオーディオの場合.
		memset( buf_l, 0, BufSize * 2 );
		memset( buf_r, 0, BufSize * 2 );
		if( WavSize[ActivePage] > 0 ){
			short*	dst_l = (short*)buf_l;
			short*	dst_r = (short*)buf_r;
			short*	src   = (short*)&(WavBuf[ActivePage][ReadCnt]);
			DWORD	limit = (WavSize[ActivePage] - ReadCnt) / 4;
			if( limit > BufSize ){
				limit = BufSize;
			}
			for( DWORD i = 0; i < limit; i++ ){
				*(dst_l++) = *(src++);
				*(dst_r++) = *(src++);
			}
			ReadCnt += (BufSize * 4);
		}
		break;

	case ASIOSTInt24LSB:
		//! 24ビットオーディオの場合.
		memset( buf_l, 0, BufSize * 3 );
		memset( buf_r, 0, BufSize * 3 );
		if( WavSize[ActivePage] > 0 ){
			BYTE*	dst_l = buf_l;
			BYTE*	dst_r = buf_r;
			BYTE*	src   = &(WavBuf[ActivePage][ReadCnt]);
			DWORD	limit = (WavSize[ActivePage] - ReadCnt) / 4;
			if( limit > BufSize ){
				limit = BufSize;
			}
			for( DWORD i = 0; i < limit; i++ ){
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
		//! 32ビットオーディオの場合.
		memset( buf_l, 0, BufSize * 4 );
		memset( buf_r, 0, BufSize * 4 );
		if( WavSize[ActivePage] > 0 ){
			short*	dst_l = (short*)buf_l;
			short*	dst_r = (short*)buf_r;
			short*	src   = (short*)&(WavBuf[ActivePage][ReadCnt]);
			DWORD	limit = (WavSize[ActivePage] - ReadCnt) / 4;
			if( limit > BufSize ){
				limit = BufSize;
			}
			for( DWORD i = 0; i < limit; i++ ){
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
		memset( buf_l, 0, BufSize * 4 );
		memset( buf_r, 0, BufSize * 4 );
		break;

	case ASIOSTFloat64LSB:
	case ASIOSTFloat64MSB: 
		memset( buf_l, 0, BufSize * 8 );
		memset( buf_r, 0, BufSize * 8 );
		break;

	case ASIOSTInt16MSB:
		memset( buf_l, 0, BufSize * 2 );
		memset( buf_r, 0, BufSize * 2 );
		break;

	case ASIOSTInt24MSB:
		memset( buf_l, 0, BufSize * 3 );
		memset( buf_r, 0, BufSize * 3 );
		break;
	}
	// ASIOOutputReady()をサポートしている場合、毎回ASIOOutputReady()を呼び出す.
	ASIOOutputReady();
}

/* End of file */
