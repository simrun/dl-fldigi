// ----------------------------------------------------------------------------
// cw.h  --  morse code modem
//
// Copyright (C) 2006-2009
//		Dave Freese, W1HKJ
//
// This file is part of fldigi.  Adapted in part from code contained in 
// gmfsk source code distribution.
//  gmfsk Copyright (C) 2001, 2002, 2003
//  Tomi Manninen (oh2bns@sral.fi)
//  Copyright (C) 2004
//  Lawrence Glaister (ve7it@shaw.ca)
//
// Fldigi is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Fldigi is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with fldigi.  If not, see <http://www.gnu.org/licenses/>.
// ----------------------------------------------------------------------------

#ifndef _CW_H
#define _CW_H

#include "modem.h"
#include "filters.h"
#include "mbuffer.h"

#define	CWSampleRate	8000
#define	CWMaxSymLen		4096
#define KNUM 			128

// decimation ratio for the receiver 
//#define	DEC_RATIO	8
//#define CW_FIRLEN 32		
//#define CW_FIRLEN   128      
//#define CW_FIRLEN	  256
//#define CW_FIRLEN   512
// Limits on values of CW send and timing parameters 
//#define	CW_MIN_SPEED		5	// Lowest WPM allowed 
//#define	CW_MAX_SPEED		100	// Highest WPM allowed 

// CW function return status codes. 
#define	CW_SUCCESS		0
#define	CW_ERROR		-1

#define	ASC_NUL		'\0'	// End of string 
#define	ASC_SPACE	' '	// ASCII space char 

// Tone and timing magic numbers. 
#define	DOT_MAGIC	1200000	// Dot length magic number.  The Dot
//							   length is 1200000/WPM Usec 
#define	TONE_SILENT	0	// 0Hz = silent 'tone' 
#define	USECS_PER_SEC	1000000	// Microseconds in a second 

#define	INITIAL_SEND_SPEED	18	// Initial send speed in WPM 
#define	INITIAL_RECEIVE_SPEED	18	// Initial receive speed in WPM 

// Initial adaptive speed threshold 
#define	INITIAL_THRESHOLD	((DOT_MAGIC / INITIAL_RECEIVE_SPEED) * 2)

// Initial noise filter threshold 
#define	INITIAL_NOISE_THRESHOLD	((DOT_MAGIC / CW_MAX_SPEED) / 2)

#define TRACKING_FILTER_SIZE 16

enum CW_RX_STATE {
	RS_IDLE = 0,
	RS_IN_TONE,
	RS_AFTER_TONE
};

enum CW_EVENT {
	CW_RESET_EVENT,
	CW_KEYDOWN_EVENT,
	CW_KEYUP_EVENT,
	CW_QUERY_EVENT
};

//class cw : public morse, public modem {
class cw : public modem {
protected:
	int				symbollen;		// length of a dot in sound samples (tx)
	int             fsymlen;        // length of extra interelement space (farnsworth)
	double			phaseacc;		// used by NCO for rx/tx tones
	unsigned int	smpl_ctr;		// sample counter for timing cw rx 
	double			agc_peak;		// threshold for tone detection 

	C_FIR_filter	*cwfilter;
	Cmovavg			*bitfilter;
	Cmovavg			*trackingfilter;

	int bitfilterlen;

	CW_RX_STATE		cw_receive_state;	// Indicates receive state 
	CW_RX_STATE		old_cw_receive_state;
	CW_EVENT		cw_event;			// functions used by cw process routine 
	 
	double pipe[CWMaxSymLen];			// storage for sync scope data
	mbuffer<double, CWMaxSymLen, 2> scopedata;
	int pipeptr;
	int pipesize;
	bool scope_clear;
	
// user configurable data - local copy passed in from gui
	int cw_speed;
	int cw_bandwidth;
	int cw_squelch;
	int cw_send_speed;				// Initially 18 WPM 
	int cw_receive_speed;			// Initially 18 WPM 
	bool usedefaultWPM;				// use default WPM
	
	int cw_upper_limit;
	int cw_lower_limit;
	
	long int cw_noise_spike_threshold;	// Initially ignore any tone < 10mS 
	int cw_in_sync;					// Synchronization flag 

// Sending parameters: 
	long int cw_send_dot_length;	// Length of a send Dot, in Usec 
	long int cw_send_dash_length;	// Length of a send Dash, in Usec
	int lastsym;					// last symbol sent
	double risetime;			    // leading/trailing edge rise time (msec)
	int knum;						// number of samples on edges
	int QSKshape;                   // leading/trailing edge shape factor
	double qskbuf[OUTBUFSIZE];		// signal array for qsk drive
	double qskphase;				//
	bool firstelement;
	
//	double *keyshape;				// array defining leading edge
	
// Receiving parameters: 
	long int cw_receive_dot_length;		// Length of a receive Dot, in Usec 
	long int cw_receive_dash_length;		// Length of a receive Dash, in Usec 

// Receive buffering
#define	RECEIVE_CAPACITY	256		// Way longer than any representation 
	char rx_rep_buf[RECEIVE_CAPACITY];
	int cw_rr_current;				// Receive buffer current location 
	unsigned int cw_rr_start_timestamp;	// Tone start timestamp 
	unsigned int cw_rr_end_timestamp;	// Tone end timestamp 

	long int cw_adaptive_receive_threshold;	// 2-dot threshold for adaptive speed 

// Receive adaptive speed tracking.
	double dot_tracking;
	double dash_tracking;
	
	inline double nco(double freq);
	inline double qsknco();
	void	update_syncscope();
	void    clear_syncscope();
	void	update_Status();
	void	sync_parameters();
	int		handle_event(int cw_event, const char **c);
	int		usec_diff(unsigned int earlier, unsigned int later);
	void	send_symbol(int symbol, int len);
	void	send_ch(int c);
	bool 	tables_init();
	unsigned int tokenize_representation(char *representation);
	void	update_tracking(int dot, int dash);
	
	void	makeshape();
	
public:
	cw();
	~cw();
	void	init();
	void	rx_init();
	void	tx_init(SoundBase *sc);
	void	restart() {};
	int		rx_process(const double *buf, int len);
	int		tx_process();
	void	incWPM();
	void	decWPM();
	void	toggleWPM();

};

#endif
