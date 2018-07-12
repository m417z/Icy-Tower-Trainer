#ifndef _BASSMOD_H_
#define _BASSMOD_H_

#include <windows.h>

typedef DWORD HSYNC;		// synchronizer handle

// Error codes returned by BASSMOD_GetErrorCode()
#define BASS_OK				0	// all is OK
#define BASS_ERROR_MEM		1	// memory error
#define BASS_ERROR_FILEOPEN	2	// can't open the file
#define BASS_ERROR_DRIVER	3	// can't find a free/valid driver
#define BASS_ERROR_HANDLE	5	// invalid handle
#define BASS_ERROR_FORMAT	6	// unsupported format
#define BASS_ERROR_POSITION	7	// invalid playback position
#define BASS_ERROR_INIT		8	// BASS_Init has not been successfully called
#define BASS_ERROR_ALREADY	14	// already initialized/loaded
#define BASS_ERROR_ILLTYPE	19	// an illegal type was specified
#define BASS_ERROR_ILLPARAM	20	// an illegal parameter was specified
#define BASS_ERROR_DEVICE	23	// illegal device number
#define BASS_ERROR_NOPLAY	24	// not playing
#define BASS_ERROR_NOMUSIC	28	// no MOD music has been loaded
#define BASS_ERROR_NOSYNC	30	// synchronizers have been disabled
#define BASS_ERROR_NOTAVAIL	37	// requested data is not available
#define BASS_ERROR_DECODE	38	// the channel is a "decoding channel"
#define BASS_ERROR_FILEFORM	41	// unsupported file format
#define BASS_ERROR_UNKNOWN	-1	// some other mystery error

// Device setup flags
#define BASS_DEVICE_8BITS	1	// use 8 bit resolution, else 16 bit
#define BASS_DEVICE_MONO	2	// use mono, else stereo
#define BASS_DEVICE_NOSYNC	16	// disable synchronizers

// Music flags
#define BASS_MUSIC_RAMP		1	// normal ramping
#define BASS_MUSIC_RAMPS	2	// sensitive ramping
#define BASS_MUSIC_LOOP		4	// loop music
#define BASS_MUSIC_FT2MOD	16	// play .MOD as FastTracker 2 does
#define BASS_MUSIC_PT1MOD	32	// play .MOD as ProTracker 1 does
#define BASS_MUSIC_POSRESET	256	// stop all notes when moving position
#define BASS_MUSIC_SURROUND	512	// surround sound
#define BASS_MUSIC_SURROUND2	1024	// surround sound (mode 2)
#define BASS_MUSIC_STOPBACK	2048	// stop the music on a backwards jump effect
#define BASS_MUSIC_CALCLEN	8192	// calculate playback length
#define BASS_MUSIC_NONINTER	16384	// non-interpolated mixing
#define BASS_MUSIC_NOSAMPLE	0x400000// don't load the samples

#define BASS_UNICODE			0x80000000

/* Sync types (with BASSMOD_MusicSetSync() "param" and SYNCPROC "data"
definitions) & flags. */
#define BASS_SYNC_MUSICPOS	0
#define BASS_SYNC_POS		0
/* Sync when the music reaches a position.
param: LOWORD=order (0=first, -1=all) HIWORD=row (0=first, -1=all)
data : LOWORD=order HIWORD=row */
#define BASS_SYNC_MUSICINST	1
/* Sync when an instrument (sample for the non-instrument based formats)
is played in the music (not including retrigs).
param: LOWORD=instrument (1=first) HIWORD=note (0=c0...119=b9, -1=all)
data : LOWORD=note HIWORD=volume (0-64) */
#define BASS_SYNC_END		2
/* Sync when the music reaches the end.
param: not used
data : 1 = the sync is triggered by a backward jump, otherwise not used */
#define BASS_SYNC_MUSICFX	3
/* Sync when the "sync" effect (XM/MTM/MOD: E8x/Wxx, IT/S3M: S2x) is used.
param: 0:data=pos, 1:data="x" value
data : param=0: LOWORD=order HIWORD=row, param=1: "x" value */
#define BASS_SYNC_ONETIME	0x80000000	// FLAG: sync only once, else continuously

typedef void (CALLBACK SYNCPROC)(HSYNC handle, DWORD data, DWORD user);
/* Sync callback function. NOTE: a sync callback function should be very
quick as other syncs can't be processed until it has finished.
handle : The sync that has occured
data   : Additional data associated with the sync's occurance */

// BASSMOD_MusicIsActive return values
#define BASS_ACTIVE_STOPPED	0
#define BASS_ACTIVE_PLAYING	1
#define BASS_ACTIVE_PAUSED	3

//extern BOOL (WINAPI* BASSMOD_Init)(int device, DWORD freq, DWORD flags);
//extern BOOL (WINAPI* BASSMOD_MusicLoad)(BOOL mem, void *file, DWORD offset, DWORD length, DWORD flags);
extern BOOL (WINAPI* BASSMOD_MusicPlay)();
extern DWORD (WINAPI* BASSMOD_MusicIsActive)();
extern BOOL (WINAPI* BASSMOD_MusicPause)();
extern void (WINAPI* BASSMOD_MusicFree)();
//extern void (WINAPI* BASSMOD_Free)();

BOOL LoadBassmod(LPCSTR lpResName, LPCSTR lpResType);
BOOL LoadBassFile(LPCSTR lpResName, LPCSTR lpResType);
void FreeBassmod();

#endif // _BASSMOD_H_
