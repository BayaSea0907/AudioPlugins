/****************************************************************************
 *	Copylight(C) 2010 Kanazawa-soft-design,LLC.All Rights Reserved.
 ****************************************************************************/
/*!
 *	@file	WaveFile.h
 *
 *	@brief	WAVEÉtÉ@ÉCÉãÇÃì«Ç›çûÇ›.
 *
 *	@author	ã‡‡V êÈñæ
 */
#ifndef	__WAVEFILE_H__
#define	__WAVEFILE_H__

BOOL			ReadOpenWaveFile( LPCTSTR file_name );
void			CloseWaveFile( void );
DWORD			ReadWaveFile( void* data, DWORD size );
BOOL			IsOpenWaveFile( void );
WAVEFORMATEX*	GetAudioFormat( WAVEFORMATEX* wf );

#endif	// __WAVEFILE_H__
/* End of file */
