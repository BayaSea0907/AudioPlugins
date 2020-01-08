/****************************************************************************
 *	Copylight(C) 2010 Kanazawa-soft-design,LLC.All Rights Reserved.
 ****************************************************************************/
/*!
 *	@file	WaveFile.cpp
 *
 *	@brief	WAVEファイルの読み込み.
 *
 *	@author	金澤 宣明
 */
#pragma once
#include "stdafx.h"
#include "WaveFile.h"

//! WAVEファイルヘッダ構造体.
typedef struct {
	DWORD		RiffId;					// RIFFファイル識別子 (RIFF).
	DWORD		FileSize;				// ファイルサイズ - 8.
	DWORD		FileType;				// ファイルタイプ ("WAVE").
	DWORD		FormatId;				// フォーマット識別子 ("fmt ").
	DWORD		FormatSize;				// フォーマット・サイズ - 8.
	WORD		FormatType;				// フォーマットタイプ.
	WORD		Channels;				// チャンネル数.
	DWORD		SamplesPerSec;			// サンプリング周期.
	DWORD		AvgBytesPerSec;			// 1秒あたりのバイト数.
	WORD		BlockAlign;				// 1チャンネルのバイト数.
	WORD		BitsPerSample;			// 1データあたりのビット数.
} WAVFILEHEADER;

//! ファクトリチャンク・ヘッダ構造体.
typedef struct {
	DWORD		Id;						// データ識別子("fact").
	DWORD		Size;					// チャンクサイズ - 8.
	DWORD		Samples;				// 全サンプル数.
} WAVEFACTCHUNK;

//! データチャンク・ヘッダ構造体.
typedef struct {
	DWORD		Id;						// データ識別子("data").
	DWORD		Size;					// データ・サイズ.
} WAVEDATACHUNK;

// ローカル変数.
static HANDLE			hFile = NULL;	// ファイル・ハンドル.
static WAVFILEHEADER	Header;			// WAVEファイルのヘッダ構造体.

#define	RIFFCC( _x )	(((_x >> 24) & 0xFF) + ((_x >> 8) & 0xFF00) + ((_x << 8) & 0xFF0000) + ((_x << 24) & 0xFF000000))

/****************************************************************************/
/*!
 *	@brief	ファイルを読み込みモードでオープンする.
 *
 *	@param	[in]	file_name	ファイル名のポインタ.
 *
 *	@retval	TRUE = OK. / FALSE = NG.
 */
BOOL	ReadOpenWaveFile( LPCTSTR file_name )
{
	DWORD offset = 0;

	//! 既にファイル・オープン中の場合、エラーにする.
	if( hFile ){
		return FALSE;
	}
	//! 引数が不正な場合、エラーにする.
	if( !file_name || _tcslen( file_name ) == 0 ){
		return FALSE;
	}
	//! ファイルを読み込みモードでオープンする.
	hFile = CreateFile( file_name, GENERIC_READ, FILE_SHARE_READ, NULL,
									OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL );
	if( hFile == INVALID_HANDLE_VALUE ){
		hFile  = NULL;
		return FALSE;
	}
	//! ファイルヘッダを解析する.
	do {
		//! ファイルの先頭からストリームを読み込む.
		BYTE	buf[256];
		DWORD	len = ReadWaveFile( buf, 256 );
		if( len == 0 ) break;

		//! ファイルヘッダを取り出す.
		Header  = *((WAVFILEHEADER*)&buf[offset]);
		offset += sizeof(Header);

		//! RIFFヘッダを調べる.
		if(( Header.RiffId   != RIFFCC( 'RIFF' ) )
		|| ( Header.FileType != RIFFCC( 'WAVE' ) )
		|| ( Header.FileSize <= (sizeof(Header) - 8) )){
			break;
		}
		//! フォーマットヘッダを調べる.
		if(( Header.FormatId       != RIFFCC( 'fmt ' ) )
		|| ( Header.Channels       ==  0 )
		|| ( Header.SamplesPerSec  ==  0 )
		|| ( Header.AvgBytesPerSec ==  0 )
		|| ( Header.BlockAlign     ==  0 )
		|| ( Header.BitsPerSample  ==  0 )
		|| ( Header.FormatSize     <  16 )){
			break;
		}
		for( ; offset < 256 && buf[offset] == 0; offset++ ) ;

		//! ファクトリチャンクを取り出す.
		WAVEFACTCHUNK fact;
		fact = *((WAVEFACTCHUNK*)&buf[offset]);
		if( fact.Id == RIFFCC( 'fact' ) ){
			offset += sizeof(WAVEFACTCHUNK);
		}else{
			fact.Samples = 0;
		}
		for( ; offset < 256 && buf[offset] == 0; offset++ ) ;

		//! データヘッダを取り出す.
		WAVEDATACHUNK data;
		data    = *((WAVEDATACHUNK*)&buf[offset]);
		offset += sizeof(WAVEDATACHUNK);

		//! データヘッダを調べる.
		if( data.Id != RIFFCC( 'data' ) || data.Size == 0 ){
			break;
		}
		//! ファイルから読み込んだデータが少ない場合、エラーにする.
		if( len <= offset ){
			break;
		}
		//! ファイルの読み込み位置を先頭フレームに合わせておく.
		LARGE_INTEGER pos1,pos2;
		pos1.QuadPart = offset;
		SetFilePointerEx( hFile, pos1, &pos2, FILE_BEGIN );

		return TRUE;

	} while( FALSE );

	//! エラーの場合、ファイルをクローズして終了する.
	CloseWaveFile();
	return FALSE;
}

/****************************************************************************/
/*!
 *	@brief	ファイル・クローズ.
 *
 *	@param	なし.
 *
 *	@retval	なし.
 */
void	CloseWaveFile( void )
{
	//! オープン中のファイルがある場合、ファイルをクローズする.
	if( hFile ){
		CloseHandle( hFile );
	}
	hFile = NULL;
}

/****************************************************************************/
/*!
 *	@brief	ファイルからWAVEデータを読み込む.
 *
 *	@param	[in]	data	WAVEデータを格納する領域のポインタ.
 *	@param	[in]	size	WAVEデータを格納する領域のバイト数.
 *
 *	@retval	ファイルから読み込んだバイト数.
 */
DWORD	ReadWaveFile( void* data, DWORD size )
{
	DWORD len = 0;

	//! ファイルからデータを読み込む.
	if( hFile ){
		if( ReadFile( hFile, data, size, &len, NULL ) ){
			return len;
		}
	}
	return 0;
}

/****************************************************************************/
/*!
 *	@brief	WAVファイルがオープン中かどうかを調べる.
 *
 *	@param	なし.
 *
 *	@retval	TRUE = ファイル・オープン中 / FALSE = ファイル・オープンしていない.
 */
BOOL	IsOpenWaveFile( void )
{
	return hFile ? TRUE : FALSE;
}

/****************************************************************************/
/*!
 *	@brief	オーディオ・フォーマットを取り出す.
 *
 *	@param	[out]	wf	オーディオ・フォーマットを格納する構造体のポインタ.
 *
 *	@retval	オーディオ・フォーマットを格納した構造体のポインタ.
 */
WAVEFORMATEX*	GetAudioFormat( WAVEFORMATEX* wf )
{
	if( wf ){
		wf->wFormatTag      = WAVE_FORMAT_PCM;
		wf->nChannels       = Header.Channels;
		wf->wBitsPerSample  = Header.BitsPerSample;
		wf->nBlockAlign     = Header.BlockAlign;
		wf->nSamplesPerSec  = Header.SamplesPerSec;
		wf->nAvgBytesPerSec = Header.AvgBytesPerSec;
		wf->cbSize          = 0;
	}
	return wf;
}

/* End of file */
