/****************************************************************************
 *	Copylight(C) 2010 Kanazawa-soft-design,LLC.All Rights Reserved.
 ****************************************************************************/
/*!
 *	@file	WaveFile.cpp
 *
 *	@brief	WAVE�t�@�C���̓ǂݍ���.
 *
 *	@author	���V �閾
 */
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include "WaveFile.h"

//! WAVE�t�@�C���w�b�_�\����.
typedef struct {
	DWORD		RiffId;					// RIFF�t�@�C�����ʎq (RIFF).
	DWORD		FileSize;				// �t�@�C���T�C�Y - 8.
	DWORD		FileType;				// �t�@�C���^�C�v ("WAVE").
	DWORD		FormatId;				// �t�H�[�}�b�g���ʎq ("fmt ").
	DWORD		FormatSize;				// �t�H�[�}�b�g�E�T�C�Y - 8.
	WORD		FormatType;				// �t�H�[�}�b�g�^�C�v.
	WORD		Channels;				// �`�����l����.
	DWORD		SamplesPerSec;			// �T���v�����O����.
	DWORD		AvgBytesPerSec;			// 1�b������̃o�C�g��.
	WORD		BlockAlign;				// 1�`�����l���̃o�C�g��.
	WORD		BitsPerSample;			// 1�f�[�^������̃r�b�g��.
} WAVFILEHEADER;

//! �t�@�N�g���`�����N�E�w�b�_�\����.
typedef struct {
	DWORD		Id;						// �f�[�^���ʎq("fact").
	DWORD		Size;					// �`�����N�T�C�Y - 8.
	DWORD		Samples;				// �S�T���v����.
} WAVEFACTCHUNK;

//! �f�[�^�`�����N�E�w�b�_�\����.
typedef struct {
	DWORD		Id;						// �f�[�^���ʎq("data").
	DWORD		Size;					// �f�[�^�E�T�C�Y.
} WAVEDATACHUNK;

// ���[�J���ϐ�.
static HANDLE			hFile = NULL;	// �t�@�C���E�n���h��.
static WAVFILEHEADER	Header;			// WAVE�t�@�C���̃w�b�_�\����.

#define	RIFFCC( _x )	(((_x >> 24) & 0xFF) + ((_x >> 8) & 0xFF00) + ((_x << 8) & 0xFF0000) + ((_x << 24) & 0xFF000000))

/****************************************************************************/
/*!
 *	@brief	�t�@�C����ǂݍ��݃��[�h�ŃI�[�v������.
 *
 *	@param	[in]	file_name	�t�@�C�����̃|�C���^.
 *
 *	@retval	TRUE = OK. / FALSE = NG.
 */
BOOL	ReadOpenWaveFile( LPCTSTR file_name )
{
	DWORD offset = 0;

	//! ���Ƀt�@�C���E�I�[�v�����̏ꍇ�A�G���[�ɂ���.
	if( hFile ){
		return FALSE;
	}
	//! �������s���ȏꍇ�A�G���[�ɂ���.
	if( !file_name || _tcslen( file_name ) == 0 ){
		return FALSE;
	}
	//! �t�@�C����ǂݍ��݃��[�h�ŃI�[�v������.
	hFile = CreateFile( file_name, GENERIC_READ, FILE_SHARE_READ, NULL,
									OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL );
	if( hFile == INVALID_HANDLE_VALUE ){
		hFile  = NULL;
		return FALSE;
	}
	//! �t�@�C���w�b�_����͂���.
	do {
		//! �t�@�C���̐擪����X�g���[����ǂݍ���.
		BYTE	buf[256];
		DWORD	len = ReadWaveFile( buf, 256 );
		if( len == 0 ) break;

		//! �t�@�C���w�b�_�����o��.
		Header  = *((WAVFILEHEADER*)&buf[offset]);
		offset += sizeof(Header);

		//! RIFF�w�b�_�𒲂ׂ�.
		if(( Header.RiffId   != RIFFCC( 'RIFF' ) )
		|| ( Header.FileType != RIFFCC( 'WAVE' ) )
		|| ( Header.FileSize <= (sizeof(Header) - 8) )){
			break;
		}
		//! �t�H�[�}�b�g�w�b�_�𒲂ׂ�.
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

		//! �t�@�N�g���`�����N�����o��.
		WAVEFACTCHUNK fact;
		fact = *((WAVEFACTCHUNK*)&buf[offset]);
		if( fact.Id == RIFFCC( 'fact' ) ){
			offset += sizeof(WAVEFACTCHUNK);
		}else{
			fact.Samples = 0;
		}
		for( ; offset < 256 && buf[offset] == 0; offset++ ) ;

		//! �f�[�^�w�b�_�����o��.
		WAVEDATACHUNK data;
		data    = *((WAVEDATACHUNK*)&buf[offset]);
		offset += sizeof(WAVEDATACHUNK);

		//! �f�[�^�w�b�_�𒲂ׂ�.
		if( data.Id != RIFFCC( 'data' ) || data.Size == 0 ){
			break;
		}
		//! �t�@�C������ǂݍ��񂾃f�[�^�����Ȃ��ꍇ�A�G���[�ɂ���.
		if( len <= offset ){
			break;
		}
		//! �t�@�C���̓ǂݍ��݈ʒu��擪�t���[���ɍ��킹�Ă���.
		LARGE_INTEGER pos1,pos2;
		pos1.QuadPart = offset;
		SetFilePointerEx( hFile, pos1, &pos2, FILE_BEGIN );

		return TRUE;

	} while( FALSE );

	//! �G���[�̏ꍇ�A�t�@�C�����N���[�Y���ďI������.
	CloseWaveFile();
	return FALSE;
}

/****************************************************************************/
/*!
 *	@brief	�t�@�C���E�N���[�Y.
 *
 *	@param	�Ȃ�.
 *
 *	@retval	�Ȃ�.
 */
void	CloseWaveFile( void )
{
	//! �I�[�v�����̃t�@�C��������ꍇ�A�t�@�C�����N���[�Y����.
	if( hFile ){
		CloseHandle( hFile );
	}
	hFile = NULL;
}

/****************************************************************************/
/*!
 *	@brief	�t�@�C������WAVE�f�[�^��ǂݍ���.
 *
 *	@param	[in]	data	WAVE�f�[�^���i�[����̈�̃|�C���^.
 *	@param	[in]	size	WAVE�f�[�^���i�[����̈�̃o�C�g��.
 *
 *	@retval	�t�@�C������ǂݍ��񂾃o�C�g��.
 */
DWORD	ReadWaveFile( void* data, DWORD size )
{
	DWORD len = 0;

	//! �t�@�C������f�[�^��ǂݍ���.
	if( hFile ){
		if( ReadFile( hFile, data, size, &len, NULL ) ){
			return len;
		}
	}
	return 0;
}

/****************************************************************************/
/*!
 *	@brief	WAV�t�@�C�����I�[�v�������ǂ����𒲂ׂ�.
 *
 *	@param	�Ȃ�.
 *
 *	@retval	TRUE = �t�@�C���E�I�[�v���� / FALSE = �t�@�C���E�I�[�v�����Ă��Ȃ�.
 */
BOOL	IsOpenWaveFile( void )
{
	return hFile ? TRUE : FALSE;
}

/****************************************************************************/
/*!
 *	@brief	�I�[�f�B�I�E�t�H�[�}�b�g�����o��.
 *
 *	@param	[out]	wf	�I�[�f�B�I�E�t�H�[�}�b�g���i�[����\���̂̃|�C���^.
 *
 *	@retval	�I�[�f�B�I�E�t�H�[�}�b�g���i�[�����\���̂̃|�C���^.
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