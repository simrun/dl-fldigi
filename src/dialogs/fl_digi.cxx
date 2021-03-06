// ----------------------------------------------------------------------------
//
//	fl_digi.cxx
//
// Copyright (C) 2006-2010
//		Dave Freese, W1HKJ
// Copyright (C) 2007-2010
//		Stelios Bounanos, M0GLD
//
// This file is part of fldigi.
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

#include <config.h>

#include <sys/types.h>

#ifdef __WOE32__
#  ifdef __CYGWIN__
#    include <w32api/windows.h>
#  else
#    include <windows.h>
#  endif
#endif

#include <cstdlib>
#include <cstdarg>
#include <string>
#include <fstream>
#include <algorithm>
#include <map>
#include <dirent.h>

#include "gettext.h"
#include "fl_digi.h"

#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Tile.H>
#include <FL/x.H>
#include <FL/Fl_Help_Dialog.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Tooltip.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Multiline_Input.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Pack.H>

#include "waterfall.h"
#include "raster.h"
#include "progress.h"

#include "main.h"
#include "threads.h"
#include "trx.h"
#if USE_HAMLIB
	#include "hamlib.h"
#endif
#include "rigio.h"
#include "rigMEM.h"
#include "psk.h"
#include "cw.h"
#include "mfsk.h"
#include "mt63.h"
#include "rtty.h"
#include "olivia.h"
#include "contestia.h"
#include "thor.h"
#include "dominoex.h"
#include "feld.h"
#include "throb.h"
#include "wwv.h"
#include "analysis.h"
#include "ssb.h"

#include "ascii.h"
#include "globals.h"
#include "misc.h"
#include "FTextRXTX.h"
#include "Fl_Tile_Check.h"

#include "confdialog.h"
#include "configuration.h"
#include "colorsfonts.h"
#include "status.h"

#include "macros.h"
#include "macroedit.h"
#include "logger.h"
#include "lookupcall.h"

#include "font_browser.h"

#include "icons.h"

#include "rigsupport.h"

#include "qrunner.h"

#include "Viewer.h"
#include "soundconf.h"

#include "htmlstrings.h"
#if USE_XMLRPC
#	include "xmlrpc.h"
#endif
#if BENCHMARK_MODE
#	include "benchmark.h"
#endif
#include "debug.h"
#include "re.h"
#include "network.h"
#include "spot.h"
#include "dxcc.h"
#include "locator.h"
#include "notify.h"

#include "logbook.h"

#include "rx_extract.h"
#include "speak.h"
#include "flmisc.h"

#include "ssdv_rx.h"

using namespace std;

//jcoxon
#include <iostream>
#include "dl_fldigi.h"
bool bHAB = false;
//

bool bWF_only = false;
bool withnoise = false;

Fl_Double_Window	*fl_digi_main      = (Fl_Double_Window *)0;
Fl_Help_Dialog 		*help_dialog       = (Fl_Help_Dialog *)0;
Fl_Double_Window	*scopeview         = (Fl_Double_Window *)0;
ssdv_rx			*ssdv              = (ssdv_rx *)0;

MixerBase* mixer = 0;

int rightof(Fl_Widget* w);
int leftof(Fl_Widget* w);
int above(Fl_Widget* w);
int below(Fl_Widget* w);

Fl_Group			*mnuFrame;
Fl_Menu_Bar 		*mnu;

Fl_Light_Button		*btnAutoSpot = (Fl_Light_Button *)0;
Fl_Light_Button		*btnTune = (Fl_Light_Button *)0;
Fl_Light_Button		*btnRSID = (Fl_Light_Button *)0;
Fl_Light_Button		*btnTxRSID = (Fl_Light_Button *)0;
Fl_Button		    *btnMacroTimer = (Fl_Button *)0;

Fl_Tile_Check		*TiledGroup = 0;
Fl_Box				*macroFrame = 0;
FTextRX				*ReceiveText = 0;
FTextTX				*TransmitText = 0;
Raster				*FHdisp;
Fl_Box				*StatusBar = (Fl_Box *)0;
Fl_Box				*Status2 = (Fl_Box *)0;
Fl_Box				*Status1 = (Fl_Box *)0;
Fl_Counter2			*cntCW_WPM=(Fl_Counter2 *)0;
Fl_Button			*btnCW_Default=(Fl_Button *)0;
Fl_Box				*WARNstatus = (Fl_Box *)0;
Fl_Button			*MODEstatus = (Fl_Button *)0;
Fl_Button 			*btnMacro[NUMMACKEYS];
Fl_Button			*btnAltMacros;
Fl_Button			*btnAFC;
Fl_Button			*btnSQL;
Fl_Input2			*inpQth;
Fl_Input2			*inpLoc;
Fl_Input2			*inpState;
Fl_Input2			*inpCountry;
Fl_Input2			*inpSerNo;
Fl_Input2			*outSerNo;
Fl_Input2			*inpXchgIn;
Fl_Input2			*inpVEprov;
Fl_Input2			*inpNotes;
Fl_Input2			*inpAZ;	// WA5ZNU
Fl_Button			*qsoTime;
Fl_Button			*btnQRZ;
Fl_Button			*qsoClear;
Fl_Button			*qsoSave;
Fl_Box				*txtRigName = (Fl_Box *)0;
cFreqControl 		*qsoFreqDisp = (cFreqControl *)0;
Fl_ComboBox			*qso_opMODE = (Fl_ComboBox *)0;
Fl_ComboBox			*qso_opBW = (Fl_ComboBox *)0;
Fl_Button			*qso_opPICK = (Fl_Button *)0;

Fl_Input2			*inpFreq;
Fl_Input2			*inpTimeOff;
Fl_Input2			*inpTimeOn;
Fl_Button			*btnTimeOn;
Fl_Input2			*inpCall;
Fl_Input2			*inpName;
Fl_Input2			*inpRstIn;
Fl_Input2			*inpRstOut;

Fl_Group			*TopFrame1 = (Fl_Group *)0;
Fl_Input2			*inpFreq1;
Fl_Input2			*inpTimeOff1;
Fl_Input2			*inpTimeOn1;
Fl_Button           *btnTimeOn1;
Fl_Input2			*inpCall1;
Fl_Input2			*inpName1;
Fl_Input2			*inpRstIn1;
Fl_Input2			*inpRstOut1;
Fl_Input2			*inpXchgIn1;
Fl_Input2			*outSerNo1;
Fl_Input2			*inpSerNo1;
cFreqControl 		*qsoFreqDisp1 = (cFreqControl *)0;

Fl_Group			*RigControlFrame = (Fl_Group *)0;
Fl_Group			*RigViewerFrame = (Fl_Group *)0;
Fl_Group			*QsoInfoFrame = (Fl_Group *)0;
Fl_Group			*QsoInfoFrame1 = (Fl_Group *)0;
Fl_Group			*QsoInfoFrame1A = (Fl_Group *)0;
Fl_Group			*QsoInfoFrame1B = (Fl_Group *)0;
Fl_Group			*QsoInfoFrameLeft = (Fl_Group *)0;
Fl_Group			*QsoInfoFrameCenter = (Fl_Group *)0;
Fl_Group			*QsoInfoFrameRight = (Fl_Group *)0;
Fl_Group			*QsoInfoFrame2 = (Fl_Group *)0;
Fl_Group			*QsoButtonFrame = (Fl_Group *)0;

Fl_Group			*TopFrame2 = (Fl_Group *)0;
cFreqControl 		*qsoFreqDisp2 = (cFreqControl *)0;
Fl_Input2			*inpFreq2;
Fl_Input2			*inpTimeOff2;
Fl_Input2			*inpTimeOn2;
Fl_Button           *btnTimeOn2;
Fl_Input2			*inpCall2;
Fl_Input2			*inpName2;
Fl_Input2			*inpRstIn2;
Fl_Input2			*inpRstOut2;
Fl_Button			*qso_opPICK2;
Fl_Button			*qsoClear2;
Fl_Button			*qsoSave2;
Fl_Button			*btnQRZ2;

Fl_Group			*TopFrame3 = (Fl_Group *)0;
cFreqControl 		*qsoFreqDisp3 = (cFreqControl *)0;
Fl_Input2			*inpTimeOff3;
Fl_Input2			*inpTimeOn3;
Fl_Button           *btnTimeOn3;
Fl_Input2			*inpCall3;
Fl_Input2			*outSerNo2;
Fl_Input2			*inpSerNo2;
Fl_Input2			*inpXchgIn2;
Fl_Button			*qso_opPICK3;
Fl_Button			*qsoClear3;
Fl_Button			*qsoSave3;

Fl_Input2			*inpCall4;

Fl_Browser			*qso_opBrowser = (Fl_Browser *)0;
Fl_Button			*qso_btnAddFreq = (Fl_Button *)0;
Fl_Button			*qso_btnSelFreq = (Fl_Button *)0;
Fl_Button			*qso_btnDelFreq = (Fl_Button *)0;
Fl_Button			*qso_btnClearList = (Fl_Button *)0;
Fl_Button			*qso_btnAct = 0;
Fl_Input2			*qso_inpAct = 0;

Fl_Group			*MixerFrame;
Fl_Value_Slider2		*valRcvMixer;
Fl_Value_Slider2		*valXmtMixer;

//jcoxon
Fl_Group			*TopFrameHAB = (Fl_Group *)0;
Fl_Output			*habTime;
Fl_Output			*habLat;
Fl_Output			*habLon;
Fl_Output			*habAlt;
Fl_Output			*habCustom=(Fl_Output *)0;
Fl_Choice			*habFlightXML = 0;
Fl_Output			*habChecksum;
Fl_Output			*habBearing;
Fl_Output			*habDistance;
Fl_Button			*habConfigureButton = 0;
Fl_Output			*habTimeSinceLastRx;

int w_habTime = 90;
int w_habLat = 90;
int w_habLon = 90;
int w_habAlt = 90;
int w_habCustom = 500;
int w_habFlightXML = 100;
int w_habConfigureButton = 120;
int w_habChecksum = 70;
int w_habBearing = 70;
int w_habDistance = 70;
int w_habTimeSinceLastRx = 100;
int HAB_height = 0;

int pad = 1;
int Hentry		= 24;
int Wbtn		= Hentry;
int x_qsoframe	= Wbtn;
int Hmenu		= 22;
int Hqsoframe	= pad + 3 * (Hentry + pad);
int Hstatus		= 22;
int Hmacros		= 22;
int w_inpFreq	= 80;
int w_inpTime	= 40;
int w_inpCall	= 120;
int w_inpName  	= 90;
int w_inpRstIn	= 30;
int w_inpRstOut = 30;
int w_SerNo		= 40;

int wf1 = pad + w_inpFreq + pad + 2*w_inpTime +  pad + w_inpCall +
          pad + w_inpName + pad + w_inpRstIn + pad + w_inpRstOut + pad;

int w_inpFreq2   = 80;
int w_inpTime2   = 40;
int w_inpCall2   = 100;
int w_inpName2   = 80;
int w_inpRstIn2  = 30;
int w_inpRstOut2 = 30;

int w_fm1 		= 25;
int w_fm2 		= 15;
int w_fm3 		= 15;
int w_fm4 		= 25;
int w_fm5 		= 25;
int w_fm6		= 30;
int w_fm7       = 35;
int w_inpState 	= 25;
int w_inpProv	= 25;
int w_inpCountry = 60;
int w_inpLOC   	= 55;
int w_inpAZ    	= 30;
int w_inpQth 	= wf1 - w_fm1 - w_fm2 - w_fm3 - w_fm4 - w_fm5 - w_fm6 -
                  w_inpState - w_inpProv - w_inpLOC - w_inpAZ - w_inpCountry;
int w_Xchg      = wf1 - 2*w_fm7 - w_fm5 - 2*pad - 2 * w_SerNo;

int qh = Hqsoframe / 2;

int IMAGE_WIDTH;
int Hwfall;
int HNOM = DEFAULT_HNOM;
// WNOM must be large enough to contain ALL of the horizontal widgets
// when the main dialog is initially created.
int WNOM = 1000;//progStatus.mainW ? progStatus.mainW : WMIN;
int Wwfall;

int					altMacros = 0;
bool				bSaveFreqList = false;
string				strMacroName[NUMMACKEYS];


waterfall			*wf = (waterfall *)0;
Digiscope			*digiscope = (Digiscope *)0;
//Digiscope			*wfscope = (Digiscope *)0;

Fl_Slider2			*sldrSquelch = (Fl_Slider2 *)0;
Progress			*pgrsSquelch = (Progress *)0;

Fl_RGB_Image		*feld_image = 0;
Fl_Pixmap 			*addrbookpixmap = 0;

#if !defined(__APPLE__) && !defined(__WOE32__) && USE_X
Pixmap				fldigi_icon_pixmap;
#endif

Fl_Menu_Item *getMenuItem(const char *caption, Fl_Menu_Item* submenu = 0);
void UI_select();
bool clean_exit(void);

void cb_init_mode(Fl_Widget *, void *arg);

void cb_oliviaA(Fl_Widget *w, void *arg);
void cb_oliviaB(Fl_Widget *w, void *arg);
void cb_oliviaC(Fl_Widget *w, void *arg);
void cb_oliviaD(Fl_Widget *w, void *arg);
void cb_oliviaE(Fl_Widget *w, void *arg);
void cb_oliviaCustom(Fl_Widget *w, void *arg);

void cb_contestiaA(Fl_Widget *w, void *arg);
void cb_contestiaB(Fl_Widget *w, void *arg);
void cb_contestiaC(Fl_Widget *w, void *arg);
void cb_contestiaD(Fl_Widget *w, void *arg);
void cb_contestiaE(Fl_Widget *w, void *arg);
void cb_contestiaF(Fl_Widget *w, void *arg);
void cb_contestiaG(Fl_Widget *w, void *arg);
void cb_contestiaH(Fl_Widget *w, void *arg);
void cb_contestiaCustom(Fl_Widget *w, void *arg);

void cb_rtty45(Fl_Widget *w, void *arg);
void cb_rtty50(Fl_Widget *w, void *arg);
void cb_rttyHAB50(Fl_Widget *w, void *arg);
void cb_rtty75(Fl_Widget *w, void *arg);
void cb_rttyCustom(Fl_Widget *w, void *arg);

Fl_Widget *modem_config_tab;
Fl_Menu_Item *quick_change;

Fl_Menu_Item quick_change_psk[] = {
	{ mode_info[MODE_PSK31].name, 0, cb_init_mode, (void *)MODE_PSK31 },
	{ mode_info[MODE_PSK63].name, 0, cb_init_mode, (void *)MODE_PSK63 },
	{ mode_info[MODE_PSK63F].name, 0, cb_init_mode, (void *)MODE_PSK63F },
	{ mode_info[MODE_PSK125].name, 0, cb_init_mode, (void *)MODE_PSK125 },
	{ mode_info[MODE_PSK250].name, 0, cb_init_mode, (void *)MODE_PSK250 },
	{ mode_info[MODE_PSK500].name, 0, cb_init_mode, (void *)MODE_PSK500 },
	{ 0 }
};

Fl_Menu_Item quick_change_qpsk[] = {
	{ mode_info[MODE_QPSK31].name, 0, cb_init_mode, (void *)MODE_QPSK31 },
	{ mode_info[MODE_QPSK63].name, 0, cb_init_mode, (void *)MODE_QPSK63 },
	{ mode_info[MODE_QPSK125].name, 0, cb_init_mode, (void *)MODE_QPSK125 },
	{ mode_info[MODE_QPSK250].name, 0, cb_init_mode, (void *)MODE_QPSK250 },
	{ mode_info[MODE_QPSK500].name, 0, cb_init_mode, (void *)MODE_QPSK500 },
	{ 0 }
};

Fl_Menu_Item quick_change_pskr[] = {
	{ mode_info[MODE_PSK125R].name, 0, cb_init_mode, (void *)MODE_PSK125R },
	{ mode_info[MODE_PSK250R].name, 0, cb_init_mode, (void *)MODE_PSK250R },
	{ mode_info[MODE_PSK500R].name, 0, cb_init_mode, (void *)MODE_PSK500R },
	{ 0 }
};

Fl_Menu_Item quick_change_mfsk[] = {
	{ mode_info[MODE_MFSK4].name, 0, cb_init_mode, (void *)MODE_MFSK4 },
	{ mode_info[MODE_MFSK8].name, 0, cb_init_mode, (void *)MODE_MFSK8 },
	{ mode_info[MODE_MFSK16].name, 0, cb_init_mode, (void *)MODE_MFSK16 },
	{ mode_info[MODE_MFSK11].name, 0, cb_init_mode, (void *)MODE_MFSK11 },
	{ mode_info[MODE_MFSK22].name, 0, cb_init_mode, (void *)MODE_MFSK22 },
	{ mode_info[MODE_MFSK31].name, 0, cb_init_mode, (void *)MODE_MFSK31 },
	{ mode_info[MODE_MFSK32].name, 0, cb_init_mode, (void *)MODE_MFSK32 },
	{ mode_info[MODE_MFSK64].name, 0, cb_init_mode, (void *)MODE_MFSK64 },
	{ 0 }
};

Fl_Menu_Item quick_change_mt63[] = {
	{ mode_info[MODE_MT63_500].name, 0, cb_init_mode, (void *)MODE_MT63_500 },
	{ mode_info[MODE_MT63_1000].name, 0, cb_init_mode, (void *)MODE_MT63_1000 },
	{ mode_info[MODE_MT63_2000].name, 0, cb_init_mode, (void *)MODE_MT63_2000 },
	{ 0 }
};

Fl_Menu_Item quick_change_thor[] = {
	{ mode_info[MODE_THOR4].name, 0, cb_init_mode, (void *)MODE_THOR4 },
	{ mode_info[MODE_THOR5].name, 0, cb_init_mode, (void *)MODE_THOR5 },
	{ mode_info[MODE_THOR8].name, 0, cb_init_mode, (void *)MODE_THOR8 },
	{ mode_info[MODE_THOR11].name, 0, cb_init_mode, (void *)MODE_THOR11 },
	{ mode_info[MODE_THOR16].name, 0, cb_init_mode, (void *)MODE_THOR16 },
	{ mode_info[MODE_THOR22].name, 0, cb_init_mode, (void *)MODE_THOR22 },
	{ 0 }
};

Fl_Menu_Item quick_change_domino[] = {
	{ mode_info[MODE_DOMINOEX4].name, 0, cb_init_mode, (void *)MODE_DOMINOEX4 },
	{ mode_info[MODE_DOMINOEX5].name, 0, cb_init_mode, (void *)MODE_DOMINOEX5 },
	{ mode_info[MODE_DOMINOEX8].name, 0, cb_init_mode, (void *)MODE_DOMINOEX8 },
	{ mode_info[MODE_DOMINOEX11].name, 0, cb_init_mode, (void *)MODE_DOMINOEX11 },
	{ mode_info[MODE_DOMINOEX16].name, 0, cb_init_mode, (void *)MODE_DOMINOEX16 },
	{ mode_info[MODE_DOMINOEX22].name, 0, cb_init_mode, (void *)MODE_DOMINOEX22 },
	{ 0 }
};

Fl_Menu_Item quick_change_feld[] = {
	{ mode_info[MODE_FELDHELL].name, 0, cb_init_mode, (void *)MODE_FELDHELL },
	{ mode_info[MODE_SLOWHELL].name, 0, cb_init_mode, (void *)MODE_SLOWHELL },
	{ mode_info[MODE_HELLX5].name,   0, cb_init_mode, (void *)MODE_HELLX5 },
	{ mode_info[MODE_HELLX9].name,   0, cb_init_mode, (void *)MODE_HELLX9 },
	{ mode_info[MODE_FSKHELL].name,  0, cb_init_mode, (void *)MODE_FSKHELL },
	{ mode_info[MODE_FSKH105].name,  0, cb_init_mode, (void *)MODE_FSKH105 },
	{ mode_info[MODE_HELL80].name,   0, cb_init_mode, (void *)MODE_HELL80 },
	{ 0 }
};

Fl_Menu_Item quick_change_throb[] = {
	{ mode_info[MODE_THROB1].name, 0, cb_init_mode, (void *)MODE_THROB1 },
	{ mode_info[MODE_THROB2].name, 0, cb_init_mode, (void *)MODE_THROB2 },
	{ mode_info[MODE_THROB4].name, 0, cb_init_mode, (void *)MODE_THROB4 },
	{ mode_info[MODE_THROBX1].name, 0, cb_init_mode, (void *)MODE_THROBX1 },
	{ mode_info[MODE_THROBX2].name, 0, cb_init_mode, (void *)MODE_THROBX2 },
	{ mode_info[MODE_THROBX4].name, 0, cb_init_mode, (void *)MODE_THROBX4 },
	{ 0 }
};

Fl_Menu_Item quick_change_olivia[] = {
	{ "8/250", 0, cb_oliviaA, (void *)MODE_OLIVIA },
	{ "8/500", 0, cb_oliviaB, (void *)MODE_OLIVIA },
	{ "16/500", 0, cb_oliviaC, (void *)MODE_OLIVIA },
	{ "8/1000", 0, cb_oliviaD, (void *)MODE_OLIVIA },
	{ "32/1000", 0, cb_oliviaE, (void *)MODE_OLIVIA },
	{ _("Custom..."), 0, cb_oliviaCustom, (void *)MODE_OLIVIA },
	{ 0 }
};

Fl_Menu_Item quick_change_contestia[] = {
	{ "4/250", 0, cb_contestiaA, (void *)MODE_CONTESTIA },
	{ "8/250", 0, cb_contestiaB, (void *)MODE_CONTESTIA },
	{ "4/500", 0, cb_contestiaC, (void *)MODE_CONTESTIA },
	{ "8/500", 0, cb_contestiaD, (void *)MODE_CONTESTIA },
	{ "16/500", 0, cb_contestiaE, (void *)MODE_CONTESTIA },
	{ "8/1000", 0, cb_contestiaF, (void *)MODE_CONTESTIA },
	{ "16/1000", 0, cb_contestiaG, (void *)MODE_CONTESTIA },
	{ "32/1000", 0, cb_contestiaH, (void *)MODE_CONTESTIA },
	{ _("Custom..."), 0, cb_contestiaCustom, (void *)MODE_CONTESTIA },
	{ 0 }
};

Fl_Menu_Item quick_change_rtty[] = {
	{ "RTTY-45", 0, cb_rtty45, (void *)MODE_RTTY },
	{ "RTTY-50", 0, cb_rtty50, (void *)MODE_RTTY },
	{ "RTTY-75", 0, cb_rtty75, (void *)MODE_RTTY },
	{ _("Custom..."), 0, cb_rttyCustom, (void *)MODE_RTTY },
	{ 0 }
};

inline int minmax(int val, int min, int max)
{
	val = val < max ? val : max;
	return val > min ? val : min;
}

// Olivia
void set_olivia_default_integ()
{
	int tones = progdefaults.oliviatones;
	int bw = progdefaults.oliviabw;

	if (tones < 1) tones = 1;
	int depth = minmax( (8 * (1 << bw)) / (1 << tones), 4, 4 * (1 << bw));

	progdefaults.oliviasinteg = depth;
	cntOlivia_sinteg->value(depth);
}

void set_olivia_tab_widgets()
{
	mnuOlivia_Bandwidth->value(progdefaults.oliviabw);
	mnuOlivia_Tones->value(progdefaults.oliviatones);
	set_olivia_default_integ();
}

void cb_oliviaA(Fl_Widget *w, void *arg)
{
	progdefaults.oliviatones = 2;
	progdefaults.oliviabw = 1;
	set_olivia_tab_widgets();
	cb_init_mode(w, arg);
}

void cb_oliviaB(Fl_Widget *w, void *arg)
{
	progdefaults.oliviatones = 2;
	progdefaults.oliviabw = 2;
	set_olivia_tab_widgets();
	cb_init_mode(w, arg);
}

void cb_oliviaC(Fl_Widget *w, void *arg)
{
	progdefaults.oliviatones = 3;
	progdefaults.oliviabw = 2;
	set_olivia_tab_widgets();
	cb_init_mode(w, arg);
}

void cb_oliviaD(Fl_Widget *w, void *arg)
{
	progdefaults.oliviatones = 2;
	progdefaults.oliviabw = 3;
	set_olivia_tab_widgets();
	cb_init_mode(w, arg);
}

void cb_oliviaE(Fl_Widget *w, void *arg)
{
	progdefaults.oliviatones = 4;
	progdefaults.oliviabw = 3;
	set_olivia_tab_widgets();
	cb_init_mode(w, arg);
}

void cb_oliviaCustom(Fl_Widget *w, void *arg)
{
	modem_config_tab = tabOlivia;
	tabsConfigure->value(tabModems);
	tabsModems->value(modem_config_tab);
#if USE_HAMLIB
	hamlib_restore_defaults();
#endif
	rigCAT_restore_defaults();;
	dlgConfig->show();
	cb_init_mode(w, arg);
}

// Contestia
void set_contestia_default_integ()
{
	int tones = progdefaults.contestiatones;
	int bw = progdefaults.contestiabw;

	if (tones < 1) tones = 1;
	int depth = minmax( (8 * (1 << bw)) / (1 << tones), 4, 4 * (1 << bw));

	progdefaults.contestiasinteg = depth;
	cntContestia_sinteg->value(depth);
}

void set_contestia_tab_widgets()
{
	mnuContestia_Bandwidth->value(progdefaults.contestiabw);
	mnuContestia_Tones->value(progdefaults.contestiatones);
	set_contestia_default_integ();
}

void cb_contestiaA(Fl_Widget *w, void *arg)
{
	progdefaults.contestiatones = 1;
	progdefaults.contestiabw = 1;
	set_contestia_tab_widgets();
	cb_init_mode(w, arg);
}

void cb_contestiaB(Fl_Widget *w, void *arg)
{
	progdefaults.contestiatones = 2;
	progdefaults.contestiabw = 1;
	set_contestia_tab_widgets();
	cb_init_mode(w, arg);
}

void cb_contestiaC(Fl_Widget *w, void *arg)
{
	progdefaults.contestiatones = 1;
	progdefaults.contestiabw = 2;
	set_contestia_tab_widgets();
	cb_init_mode(w, arg);
}

void cb_contestiaD(Fl_Widget *w, void *arg)
{
	progdefaults.contestiatones = 2;
	progdefaults.contestiabw = 2;
	set_contestia_tab_widgets();
	cb_init_mode(w, arg);
}

void cb_contestiaE(Fl_Widget *w, void *arg)
{
	progdefaults.contestiatones = 3;
	progdefaults.contestiabw = 2;
	set_contestia_tab_widgets();
	cb_init_mode(w, arg);
}

void cb_contestiaF(Fl_Widget *w, void *arg)
{
	progdefaults.contestiatones = 2;
	progdefaults.contestiabw = 3;
	set_contestia_tab_widgets();
	cb_init_mode(w, arg);
}

void cb_contestiaG(Fl_Widget *w, void *arg)
{
	progdefaults.contestiatones = 3;
	progdefaults.contestiabw = 3;
	set_contestia_tab_widgets();
	cb_init_mode(w, arg);
}

void cb_contestiaH(Fl_Widget *w, void *arg)
{
	progdefaults.contestiatones = 4;
	progdefaults.contestiabw = 3;
	set_contestia_tab_widgets();
	cb_init_mode(w, arg);
}

void cb_contestiaCustom(Fl_Widget *w, void *arg)
{
	modem_config_tab = tabContestia;
	tabsConfigure->value(tabModems);
	tabsModems->value(modem_config_tab);
#if USE_HAMLIB
	hamlib_restore_defaults();
#endif
	rigCAT_restore_defaults();;
	dlgConfig->show();
	cb_init_mode(w, arg);
}

//

void set_rtty_tab_widgets()
{
	progdefaults.rtty_parity = 0;
	progdefaults.rtty_stop = 1;
	selShift->value(progdefaults.rtty_shift);
	selCustomShift->deactivate();
	selBits->value(progdefaults.rtty_bits);
	selBaud->value(progdefaults.rtty_baud);
	selParity->value(progdefaults.rtty_parity);
	selStopBits->value(progdefaults.rtty_stop);
}

void cb_rtty45(Fl_Widget *w, void *arg)
{
	progdefaults.rtty_baud = 1;
	progdefaults.rtty_bits = 0;
	progdefaults.rtty_shift = 3;
	set_rtty_tab_widgets();
	cb_init_mode(w, arg);
}

void cb_rtty50(Fl_Widget *w, void *arg)
{
	progdefaults.rtty_baud = 2;
	progdefaults.rtty_bits = 0;
	progdefaults.rtty_shift = 3;
	set_rtty_tab_widgets();
	cb_init_mode(w, arg);
}

void cb_rttyHAB50(Fl_Widget *w, void *arg)
{
	progdefaults.rtty_baud = 2;
	progdefaults.rtty_bits = 2;
	progdefaults.rtty_shift = 8;
	progdefaults.rtty_parity = 0;
	progdefaults.rtty_stop = 1;
	set_rtty_tab_widgets();
	cb_init_mode(w, arg);
}

void cb_rtty75(Fl_Widget *w, void *arg)
{
	progdefaults.rtty_baud = 4;
	progdefaults.rtty_bits = 0;
	progdefaults.rtty_shift = 9;
	set_rtty_tab_widgets();
	cb_init_mode(w, arg);
}

void cb_rttyCustom(Fl_Widget *w, void *arg)
{
	modem_config_tab = tabRTTY;
	tabsConfigure->value(tabModems);
	tabsModems->value(modem_config_tab);
#if USE_HAMLIB
	hamlib_restore_defaults();
#endif
	rigCAT_restore_defaults();
	dlgConfig->show();

	cb_init_mode(w, arg);
}

static void busy_cursor(void*)
{
	Fl::first_window()->cursor(FL_CURSOR_WAIT);
}
static void default_cursor(void*)
{
	Fl::first_window()->cursor(FL_CURSOR_DEFAULT);
}

void startup_modem(modem* m, int f)
{
	trx_start_modem(m, f);
#if BENCHMARK_MODE
	return;
#endif

	restoreFocus();

	trx_mode id = m->get_mode();

	if (!bHAB)
	{
		if (id == MODE_CW) {
			cntCW_WPM->show();
			btnCW_Default->show();
			Status1->hide();
		} else {
			cntCW_WPM->hide();
			btnCW_Default->hide();
			Status1->show();
		}
	}

	if (id >= MODE_HELL_FIRST && id <= MODE_HELL_LAST) {
		ReceiveText->hide();
		FHdisp->show();
		sldrHellBW->value(progdefaults.HELL_BW);
	}
//jcoxon 
/*
	else if (!bWF_only) {
		ReceiveText->show();
		FHdisp->hide();
	}
	*/
	else if (!bWF_only && !bHAB) {
		ReceiveText->show();
		FHdisp->hide();
	}
//
	if (id == MODE_RTTY)
	    sldrRTTYbandwidth->value(progdefaults.RTTY_BW);
	else if (id >= MODE_PSK_FIRST && id <= MODE_PSK_LAST)
		m->set_sigsearch(SIGSEARCH);

	if (m->get_cap() & modem::CAP_AFC) {
		btnAFC->value(progStatus.afconoff);
		btnAFC->activate();
	}
	else {
		btnAFC->value(0);
		btnAFC->deactivate();
	}

	if (m->get_cap() & modem::CAP_REV) {
		wf->btnRev->value(wf->Reverse());
		wf->btnRev->activate();
	}
	else {
		wf->btnRev->value(0);
		wf->btnRev->deactivate();
	}
}

void cb_mnuOpenMacro(Fl_Menu_*, void*) {
	if (macros.changed) {
		switch (fl_choice2(_("Save changed macros?"), _("Cancel"), _("Save"), _("Don't save"))) {
		case 0:
			return;
		case 1:
			macros.saveMacroFile();
			// fall through
		case 2:
			break;
		}
	}
	macros.openMacroFile();
	macros.changed = false;
	restoreFocus();
}

void cb_mnuSaveMacro(Fl_Menu_*, void*) {
	macros.saveMacroFile();
	restoreFocus();
}

void cb_E(Fl_Menu_*, void*) {
	fl_digi_main->do_callback();
}

void cb_wMain(Fl_Widget*, void*)
{
	if (!clean_exit())
		return;
	// hide all shown windows
	Fl::first_window(fl_digi_main);
	for (Fl_Window* w = Fl::next_window(fl_digi_main); w; w = Fl::next_window(w)) {
		w->do_callback();
		w = fl_digi_main;
	}
	// this will make Fl::run return
	fl_digi_main->hide();
}

void init_modem(trx_mode mode, int freq)
{
	ENSURE_THREAD(FLMAIN_TID);

#if !BENCHMARK_MODE
       quick_change = 0;
       modem_config_tab = tabsModems->child(0);
#endif

	switch (mode) {
	case MODE_NEXT:
		if ((mode = active_modem->get_mode() + 1) == NUM_MODES)
			mode = 0;
		return init_modem(mode, freq);
	case MODE_PREV:
		if ((mode = active_modem->get_mode() - 1) < 0)
			mode = NUM_MODES - 1;
		return init_modem(mode, freq);

	case MODE_CW:
		startup_modem(*mode_info[mode].modem ? *mode_info[mode].modem :
			      *mode_info[mode].modem = new cw, freq);
		modem_config_tab = tabCW;
		break;

	case MODE_THOR4: case MODE_THOR5: case MODE_THOR8:
	case MODE_THOR11:case MODE_THOR16: case MODE_THOR22:
		startup_modem(*mode_info[mode].modem ? *mode_info[mode].modem :
			      *mode_info[mode].modem = new thor(mode), freq);
		quick_change = quick_change_thor;
		modem_config_tab = tabTHOR;
		break;

	case MODE_DOMINOEX4: case MODE_DOMINOEX5: case MODE_DOMINOEX8:
	case MODE_DOMINOEX11: case MODE_DOMINOEX16: case MODE_DOMINOEX22:
		startup_modem(*mode_info[mode].modem ? *mode_info[mode].modem :
			      *mode_info[mode].modem = new dominoex(mode), freq);
		quick_change = quick_change_domino;
		modem_config_tab = tabDomEX;
		break;

	case MODE_FELDHELL:
	case MODE_SLOWHELL:
	case MODE_HELLX5:
	case MODE_HELLX9:
	case MODE_FSKHELL:
	case MODE_FSKH105:
	case MODE_HELL80:
		startup_modem(*mode_info[mode].modem ? *mode_info[mode].modem :
			      *mode_info[mode].modem = new feld(mode), freq);
		quick_change = quick_change_feld;
		modem_config_tab = tabFeld;
		break;

	case MODE_MFSK4:
	case MODE_MFSK11:
	case MODE_MFSK22:
	case MODE_MFSK31:
	case MODE_MFSK64:
	case MODE_MFSK8:
	case MODE_MFSK16:
	case MODE_MFSK32:
		startup_modem(*mode_info[mode].modem ? *mode_info[mode].modem :
			      *mode_info[mode].modem = new mfsk(mode), freq);
		quick_change = quick_change_mfsk;
		break;

	case MODE_MT63_500: case MODE_MT63_1000: case MODE_MT63_2000 :
		startup_modem(*mode_info[mode].modem ? *mode_info[mode].modem :
			      *mode_info[mode].modem = new mt63(mode), freq);
		quick_change = quick_change_mt63;
		modem_config_tab = tabMT63;
		break;

	case MODE_PSK31: case MODE_PSK63: case MODE_PSK63F:
	case MODE_PSK125: case MODE_PSK250: case MODE_PSK500:
		startup_modem(*mode_info[mode].modem ? *mode_info[mode].modem :
			      *mode_info[mode].modem = new psk(mode), freq);
		quick_change = quick_change_psk;
		modem_config_tab = tabPSK;
		break;
	case MODE_QPSK31: case MODE_QPSK63: case MODE_QPSK125: case MODE_QPSK250: case MODE_QPSK500:
		startup_modem(*mode_info[mode].modem ? *mode_info[mode].modem :
			      *mode_info[mode].modem = new psk(mode), freq);
		quick_change = quick_change_qpsk;
		modem_config_tab = tabPSK;
		break;
	case MODE_PSK125R: case MODE_PSK250R: case MODE_PSK500R:
		startup_modem(*mode_info[mode].modem ? *mode_info[mode].modem :
			      *mode_info[mode].modem = new psk(mode), freq);
		quick_change = quick_change_pskr;
		modem_config_tab = tabPSK;
		break;

	case MODE_OLIVIA:
		startup_modem(*mode_info[mode].modem ? *mode_info[mode].modem :
			      *mode_info[mode].modem = new olivia, freq);
		modem_config_tab = tabOlivia;
		quick_change = quick_change_olivia;
		break;

	case MODE_CONTESTIA:
		startup_modem(*mode_info[mode].modem ? *mode_info[mode].modem :
			      *mode_info[mode].modem = new contestia, freq);
		modem_config_tab = tabContestia;
		quick_change = quick_change_contestia;
		break;

	case MODE_RTTY:
		startup_modem(*mode_info[mode].modem ? *mode_info[mode].modem :
			      *mode_info[mode].modem = new rtty(mode), freq);
		modem_config_tab = tabRTTY;
		quick_change = quick_change_rtty;
		break;

	case MODE_THROB1: case MODE_THROB2: case MODE_THROB4:
	case MODE_THROBX1: case MODE_THROBX2: case MODE_THROBX4:
		startup_modem(*mode_info[mode].modem ? *mode_info[mode].modem :
			      *mode_info[mode].modem = new throb(mode), freq);
		quick_change = quick_change_throb;
		break;

	case MODE_WWV:
		startup_modem(*mode_info[mode].modem ? *mode_info[mode].modem :
			      *mode_info[mode].modem = new wwv, freq);
		break;

	case MODE_ANALYSIS:
		startup_modem(*mode_info[mode].modem ? *mode_info[mode].modem :
			      *mode_info[mode].modem = new anal, freq);
		break;

	case MODE_SSB:
		startup_modem(*mode_info[mode].modem ? *mode_info[mode].modem :
			      *mode_info[mode].modem = new ssb, freq);
		break;

	default:
		LOG_ERROR("Unknown mode: %" PRIdPTR, mode);
		return init_modem(MODE_PSK31, freq);
	}

#if BENCHMARK_MODE
	return;
#endif

	clear_StatusMessages();
	progStatus.lastmode = mode;

	if (wf->xmtlock->value() == 1 && !mailserver) {
		wf->xmtlock->value(0);
		wf->xmtlock->damage();
		active_modem->set_freqlock(false);
	}
}

void init_modem_sync(trx_mode m, int f)
{
	ENSURE_THREAD(FLMAIN_TID);

	if (trx_state != STATE_RX)
		TRX_WAIT(STATE_RX, abort_tx());

	TRX_WAIT(STATE_RX, init_modem(m, f));
	REQ_FLUSH(TRX_TID);
}

void cb_init_mode(Fl_Widget *, void *mode)
{
	init_modem(reinterpret_cast<trx_mode>(mode));
}


void restoreFocus(Fl_Widget* w)
{
	// if w is not NULL, give focus to TransmitText only if the last event
	// was an Enter keypress
	if (!w)
		TransmitText->take_focus();
	else if (Fl::event() == FL_KEYBOARD) {
		int k = Fl::event_key();
		if (k == FL_Enter || k == FL_KP_Enter)
			TransmitText->take_focus();
	}
}

void macro_cb(Fl_Widget *w, void *v)
{
	int b = (int)(reinterpret_cast<long> (v));
	b += altMacros * NUMMACKEYS;
	int mouse = Fl::event_button();
	if (mouse == FL_LEFT_MOUSE && !macros.text[b].empty()) {
		stopMacroTimer();
		macros.execute(b);
	}
	else if (mouse == FL_RIGHT_MOUSE)
		editMacro(b);
	restoreFocus();
}

void colorize_macro(int i)
{
	if (progdefaults.useGroupColors == true) {
		if (i < NUMKEYROWS){
			btnMacro[i]->color(fl_rgb_color(
				progdefaults.btnGroup1.R,
				progdefaults.btnGroup1.G,
				progdefaults.btnGroup1.B));
		} else if (i < 8) {
			btnMacro[i]->color(fl_rgb_color(
				progdefaults.btnGroup2.R,
				progdefaults.btnGroup2.G,
				progdefaults.btnGroup2.B));
		} else {
			btnMacro[i]->color(fl_rgb_color(
				progdefaults.btnGroup3.R,
				progdefaults.btnGroup3.G,
				progdefaults.btnGroup3.B));
		}
		btnMacro[i]->labelcolor(
			fl_rgb_color(
				progdefaults.btnFkeyTextColor.R,
				progdefaults.btnFkeyTextColor.G,
				progdefaults.btnFkeyTextColor.B ));
	} else {
		btnMacro[i]->color(FL_BACKGROUND_COLOR);
		btnMacro[i]->labelcolor(FL_FOREGROUND_COLOR);
	}
}

void colorize_macros()
{
	FL_LOCK_D();
	for (int i = 0; i < NUMMACKEYS; i++) {
		colorize_macro(i);
		btnMacro[i]->redraw_label();
	}
	FL_UNLOCK_D();
}

void altmacro_cb(Fl_Widget *w, void *v)
{
	static char alt_text[NUMKEYROWS];

	intptr_t arg = reinterpret_cast<intptr_t>(v);
	if (arg)
		altMacros += arg;
	else
		altMacros = altMacros + (Fl::event_button() == FL_RIGHT_MOUSE ? -1 : 1);
	altMacros = WCLAMP(altMacros, 0, 3);

	snprintf(alt_text, sizeof(alt_text), "%d", altMacros + 1);
	FL_LOCK_D();
	for (int i = 0; i < NUMMACKEYS; i++)
		btnMacro[i]->label(macros.name[i + (altMacros * NUMMACKEYS)].c_str());
	btnAltMacros->label(alt_text);
	btnAltMacros->redraw_label();
	FL_UNLOCK_D();
	restoreFocus();
}

void cb_mnuConfigOperator(Fl_Menu_*, void*) {
	progdefaults.loadDefaults();
	tabsConfigure->value(tabOperator);
#if USE_HAMLIB
	hamlib_restore_defaults();
#endif
	rigCAT_restore_defaults();
	dlgConfig->show();

}

void cb_mnuConfigWaterfall(Fl_Menu_*, void*) {
	progdefaults.loadDefaults();
	tabsConfigure->value(tabWaterfall);
#if USE_HAMLIB
	hamlib_restore_defaults();
#endif
	rigCAT_restore_defaults();
	dlgConfig->show();

}

void cb_mnuConfigID(Fl_Menu_*, void*) {
	progdefaults.loadDefaults();
	tabsConfigure->value(tabID);
#if USE_HAMLIB
	hamlib_restore_defaults();
#endif
	rigCAT_restore_defaults();
	dlgConfig->show();

}

void cb_mnuConfigQRZ(Fl_Menu_*, void*) {
	progdefaults.loadDefaults();
	tabsConfigure->value(tabQRZ);
#if USE_HAMLIB
	hamlib_restore_defaults();
#endif
	rigCAT_restore_defaults();
	dlgConfig->show();

}

void cb_mnuConfigMisc(Fl_Menu_*, void*) {
	progdefaults.loadDefaults();
	tabsConfigure->value(tabMisc);
#if USE_HAMLIB
	hamlib_restore_defaults();
#endif
	rigCAT_restore_defaults();
	dlgConfig->show();

}

void cb_mnuConfigNotify(Fl_Menu_*, void*)
{
	notify_show();
}

void cb_mnuUI(Fl_Menu_*, void *) {
	progdefaults.loadDefaults();
	tabsConfigure->value(tabUI);
#if USE_HAMLIB
	hamlib_restore_defaults();
#endif
	rigCAT_restore_defaults();
	dlgConfig->show();

}

void cb_mnuConfigContest(Fl_Menu_*, void*) {
	progdefaults.loadDefaults();
	tabsConfigure->value(tabUI);
	tabsUI->value(tabContest);
#if USE_HAMLIB
	hamlib_restore_defaults();
#endif
	rigCAT_restore_defaults();
	dlgConfig->show();

}

void cb_mnuConfigRigCtrl(Fl_Menu_*, void*) {
	progdefaults.loadDefaults();
	tabsConfigure->value(tabRig);
#if USE_HAMLIB
	hamlib_restore_defaults();
#endif
	rigCAT_restore_defaults();
	dlgConfig->show();

}

void cb_mnuConfigSoundCard(Fl_Menu_*, void*) {
	progdefaults.loadDefaults();
	tabsConfigure->value(tabSoundCard);
#if USE_HAMLIB
	hamlib_restore_defaults();
#endif
	rigCAT_restore_defaults();
	dlgConfig->show();

}

void cb_mnuConfigModems(Fl_Menu_*, void*) {
	progdefaults.loadDefaults();
	tabsConfigure->value(tabModems);
#if USE_HAMLIB
	hamlib_restore_defaults();
#endif
	rigCAT_restore_defaults();
	dlgConfig->show();

}

void cb_mnuConfigWFcontrols(Fl_Menu_ *, void*) {
	progdefaults.loadDefaults();
	tabsConfigure->value(tabUI);
	tabsUI->value(tabWF_UI);
#if USE_HAMLIB
	hamlib_restore_defaults();
#endif
	rigCAT_restore_defaults();
	dlgConfig->show();

}

void cb_dl_fldigi_refresh(Fl_Widget *, void *)
{
	/* Force refresh */
	dl_fldigi_download();
	dl_fldigi_downloaded_once = 1;
}

void cb_toggle_dl_online(Fl_Widget *, void *)
{
	progdefaults.loadDefaults();
	cb_dl_fldigi_toggle_dl_online();
	confdialog_dl_online->value(progdefaults.dl_online);
}

//jcoxon added 21/3/10

void cb_mnuConfigDLClient(Fl_Menu_*, void*) {
	progdefaults.loadDefaults();
	tabsConfigure->value(tabDL);
	dlgConfig->show();
}

void cb_mnuVisitDLClient(Fl_Widget*, void*)
{
	cb_mnuVisitURL(0, (void*)string("http://ukhas.org.uk/guides:tracking_guide").c_str());
}

void cb_mnuVisitTracker(Fl_Widget*, void*)
{
	cb_mnuVisitURL(0, (void*)string("http://spacenear.us/tracker").c_str());
}

void cb_mnuVisitView(Fl_Widget*, void*)
{
	cb_mnuVisitURL(0, (void*)string("http://www.robertharrison.org/listen/view.php").c_str());
}

//End

void cb_logfile(Fl_Widget* w, void*)
{
	progStatus.LOGenabled = reinterpret_cast<Fl_Menu_*>(w)->mvalue()->value();
	if (progStatus.LOGenabled == true) {
    	Date tdy;
	    string lfname = HomeDir;
	    lfname.append("fldigi");
	    lfname.append(tdy.szDate(2));
	    lfname.append(".log");
	   	logfile = new cLogfile(lfname);
    	logfile->log_to_file_start();
    } else {
        logfile->log_to_file_stop();
        delete logfile;
        logfile = 0;
    }
}

#if USE_SNDFILE
bool capval = false;
bool genval = false;
bool playval = false;
void cb_mnuCapture(Fl_Widget *w, void *d)
{
	if (!scard) return;
	Fl_Menu_Item *m = getMenuItem(((Fl_Menu_*)w)->mvalue()->label()); //eek
	if (playval || genval) {
		m->clear();
		return;
	}
	capval = m->value();
	if(!scard->Capture(capval)) {
		m->clear();
		capval = false;
	}
}

void cb_mnuGenerate(Fl_Widget *w, void *d)
{
	if (!scard) return;
	Fl_Menu_Item *m = getMenuItem(((Fl_Menu_*)w)->mvalue()->label());
	if (capval || playval) {
		m->clear();
		return;
	}
	genval = m->value();
	if (!scard->Generate(genval)) {
		m->clear();
		genval = false;
	}
}

void cb_mnuPlayback(Fl_Widget *w, void *d)
{
	cout << "Playback Selected" << endl;
	if (!scard) {
	 return;
	 }
		Fl_Menu_Item *m = getMenuItem(((Fl_Menu_*)w)->mvalue()->label());

	if (capval || genval) {
		m->clear();
		return;
	}
	playval = m->value();
	cout << playval << endl;
	if(!scard->Playback(playval)) {
		m->clear();
		playval = false;
	}
	else if (btnAutoSpot->value()) {
		put_status(_("Spotting disabled"), 3.0);
		btnAutoSpot->value(0);
		btnAutoSpot->do_callback();
	}
}
#endif // USE_SNDFILE

void cb_mnuConfigFonts(Fl_Menu_*, void *) {
	selectColorsFonts();
}

void cb_mnuSaveConfig(Fl_Menu_ *, void *) {
	progdefaults.saveDefaults();
	restoreFocus();
}

// This function may be called by the QRZ thread
void cb_mnuVisitURL(Fl_Widget*, void* arg)
{
	const char* url = reinterpret_cast<const char *>(arg);
#ifndef __WOE32__
	const char* browsers[] = {
#  ifdef __APPLE__
		getenv("FLDIGI_BROWSER"), // valid for any OS - set by user
		"open"                    // OS X
#  else
		"fl-xdg-open",            // Puppy Linux
		"xdg-open",               // other Unix-Linux distros
		getenv("FLDIGI_BROWSER"), // force use of spec'd browser
		getenv("BROWSER"),        // most Linux distributions
		"sensible-browser",
		"firefox",
		"mozilla"                 // must be something out there!
#  endif
	};
	switch (fork()) {
	case 0:
#  ifndef NDEBUG
		unsetenv("MALLOC_CHECK_");
		unsetenv("MALLOC_PERTURB_");
#  endif
		for (size_t i = 0; i < sizeof(browsers)/sizeof(browsers[0]); i++)
			if (browsers[i])
				execlp(browsers[i], browsers[i], url, (char*)0);
		exit(EXIT_FAILURE);
	case -1:
		fl_alert2(_("Could not run a web browser:\n%s\n\n"
			 "Open this URL manually:\n%s"),
			 strerror(errno), url);
	}
#else
	// gurgle... gurgle... HOWL
	// "The return value is cast as an HINSTANCE for backward
	// compatibility with 16-bit Windows applications. It is
	// not a true HINSTANCE, however. The only thing that can
	// be done with the returned HINSTANCE is to cast it to an
	// int and compare it with the value 32 or one of the error
	// codes below." (Error codes omitted to preserve sanity).
	if ((int)ShellExecute(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL) <= 32)
		fl_alert2(_("Could not open url:\n%s\n"), url);
#endif
}

void cb_mnuVisitPSKRep(Fl_Widget*, void*)
{
	cb_mnuVisitURL(0, (void*)string("http://pskreporter.info/pskmap?").append(progdefaults.myCall).c_str());
}

void html_help( const string &Html)
{
	if (!help_dialog)
		help_dialog = new Fl_Help_Dialog;
	help_dialog->value(Html.c_str());
	help_dialog->show();
}

void cb_mnuBeginnersURL(Fl_Widget*, void*)
{
	string deffname = HelpDir;
	deffname.append("beginners.html");
	ofstream f(deffname.c_str());
	if (!f)
		return;
	f << szBeginner;
	f.close();
#ifndef __WOE32__
	cb_mnuVisitURL(NULL, (void *)deffname.insert(0, "file://").c_str());
#else
	cb_mnuVisitURL(NULL, (void *)deffname.c_str());
#endif
}

void cb_mnuCheckUpdate(Fl_Widget*, void*)
{
	struct {
		const char* url;
		const char* re;
		string version_str;
		unsigned long version;
	} sites[] = {
		{ PACKAGE_DL, "fldigi-distro/fldigi-([0-9.]+).tar.gz", "", 0 },
		{ PACKAGE_PROJ, "fldigi/fldigi-([0-9.]+).tar.gz", "", 0 }
	}, *latest;
	string reply;

	put_status(_("Checking for updates..."));
	for (size_t i = 0; i < sizeof(sites)/sizeof(*sites); i++) { // fetch .url, grep for .re
		reply.clear();
		if (!fetch_http_gui(sites[i].url, reply, 20.0, busy_cursor, 0, default_cursor, 0))
			continue;
		re_t re(sites[i].re, REG_EXTENDED | REG_ICASE | REG_NEWLINE);
		if (!re.match(reply.c_str()) || re.nsub() != 2)
			continue;

		sites[i].version = ver2int((sites[i].version_str = re.submatch(1)).c_str());
	}
	put_status("");

	latest = sites[1].version > sites[0].version ? &sites[1] : &sites[0];
	if (sites[0].version == 0 && sites[1].version == 0) {
		fl_alert2(_("Could not check for updates:\n%s"), reply.c_str());
		return;
	}
	if (latest->version > ver2int(PACKAGE_VERSION)) {
		switch (fl_choice2(_("Version %s is available at\n\n%s\n\nWhat would you like to do?"),
				  _("Close"), _("Visit URL"), _("Copy URL"),
				  latest->version_str.c_str(), latest->url)) {
		case 1:
			cb_mnuVisitURL(NULL, (void*)latest->url);
			break;
		case 2:
			size_t n = strlen(latest->url);
			Fl::copy(latest->url, n, 0);
			Fl::copy(latest->url, n, 1);
		}
	}
	else
		fl_message2(_("You are running the latest version"));
}

void cb_mnuAboutURL(Fl_Widget*, void*)
{
	if (!help_dialog)
		help_dialog = new Fl_Help_Dialog;
	help_dialog->value(szAbout);
	help_dialog->resize(help_dialog->x(), help_dialog->y(), help_dialog->w(), 440);
	help_dialog->show();
}

void fldigi_help(const string& theHelp)
{
	string htmlHelp =
"<HTML>"
"<HEAD>"
"<TITLE>" PACKAGE " Help</TITLE>"
"</HEAD>"
"<BODY>"
"<FONT FACE=fixed>"
"<P><TT>";

	for (size_t i = 0; i < theHelp.length(); i++) {
		if (theHelp[i] == '\n') {
			if (theHelp[i+1] == '\n') {
				htmlHelp += "</TT></P><P><TT>";
				i++;
			}
			else
				htmlHelp += "<BR>";
		} else if (theHelp[i] == ' ' && theHelp[i+1] == ' ') {
			htmlHelp += "&nbsp;&nbsp;";
			i++;
		} else
			htmlHelp += theHelp[i];
	}
	htmlHelp +=
"</TT></P>"
"</BODY>"
"</HTML>";
	html_help(htmlHelp);
}

void cb_mnuCmdLineHelp(Fl_Widget*, void*)
{
	extern string option_help;
	fldigi_help(option_help);
	restoreFocus();
}

void cb_mnuBuildInfo(Fl_Widget*, void*)
{
	extern string build_text;
	fldigi_help(build_text);
	restoreFocus();
}

void cb_mnuDebug(Fl_Widget*, void*)
{
	debug::show();
}

#ifndef NDEBUG
void cb_mnuFun(Fl_Widget*, void*)
{
        fl_message2(_("Sunspot creation underway!"));
}
#endif

void cb_mnuAudioInfo(Fl_Widget*, void*)
{
        if (progdefaults.btnAudioIOis != SND_IDX_PORT) {
                fl_alert2(_("Audio device information is only available for the PortAudio backend"));
                return;
        }

#if USE_PORTAUDIO
	size_t ndev;
        string devtext[2], headers[2];
	SoundPort::devices_info(devtext[0], devtext[1]);
	if (devtext[0] != devtext[1]) {
		headers[0] = _("Capture device");
		headers[1] = _("Playback device");
		ndev = 2;
	}
	else {
		headers[0] = _("Capture and playback devices");
		ndev = 1;
	}

	string audio_info;
	for (size_t i = 0; i < ndev; i++) {
		audio_info.append("<center><h4>").append(headers[i]).append("</h4>\n<table border=\"1\">\n");

		string::size_type j, n = 0;
		while ((j = devtext[i].find(": ", n)) != string::npos) {
			audio_info.append("<tr>")
				  .append("<td align=\"center\">")
				  .append(devtext[i].substr(n, j-n))
				  .append("</td>");

			if ((n = devtext[i].find('\n', j)) == string::npos) {
				devtext[i] += '\n';
				n = devtext[i].length() - 1;
			}

			audio_info.append("<td align=\"center\">")
				  .append(devtext[i].substr(j+2, n-j-2))
				  .append("</td>")
				  .append("</tr>\n");
		}
		audio_info.append("</table></center><br>\n");
	}

	fldigi_help(audio_info);
#endif
}

void cb_ShowConfig(Fl_Widget*, void*)
{
	cb_mnuVisitURL(0, (void*)HomeDir.c_str());
}

void cb_ShowNBEMS(Fl_Widget*, void*)
{
	DIR *nbems_dir;
	nbems_dir = opendir(NBEMS_dir.c_str());
	if (!nbems_dir) {
		int ans = fl_choice2(_("Do not exist, create?"), _("No"), _("Yes"), 0);
		if (!ans) return;
		check_nbems_dirs();
	}
	closedir(nbems_dir);
	cb_mnuVisitURL(0, (void*)NBEMS_dir.c_str());
}

void cbTune(Fl_Widget *w, void *) {
	Fl_Button *b = (Fl_Button *)w;
	if (!(active_modem->get_cap() & modem::CAP_TX)) {
		b->value(0);
		return;
	}
	if (b->value() == 1) {
		b->labelcolor(FL_RED);
		trx_tune();
	} else {
		b->labelcolor(FL_FOREGROUND_COLOR);
		trx_receive();
	}
	restoreFocus();
}

void cbRSID(Fl_Widget *w, void *)
{
	progdefaults.rsid = btnRSID->value();
	progdefaults.changed = true;
	restoreFocus();
}

void cbTxRSID(Fl_Widget *w, void*)
{
	progdefaults.TransmitRSid = btnTxRSID->value();
	progdefaults.changed = true;
	restoreFocus();
}

void cbAutoSpot(Fl_Widget* w, void*)
{
	progStatus.spot_recv = static_cast<Fl_Light_Button*>(w)->value();
}

void toggleRSID()
{
	btnRSID->value(0);
	cbRSID(NULL, NULL);
}

void cb_mnuDigiscope(Fl_Menu_ *w, void *d) {
	if (scopeview)
		scopeview->show();
}

void cb_mnuViewer(Fl_Menu_ *, void *) {
	openViewer();
}

void cb_mnuShowSSDVRX(Fl_Menu_ *, void *) {
	if (ssdv)
	{
		ssdv->show();
	}
}

void cb_mnuShowCountries(Fl_Menu_ *, void *)
{
	notify_dxcc_show();
}

void cb_mnuContest(Fl_Menu_ *m, void *) {
	if (QsoInfoFrame1A->visible()) {
		QsoInfoFrame1A->hide();
		QsoInfoFrame1B->show();
	} else {
		QsoInfoFrame1B->hide();
		QsoInfoFrame1A->show();
	}
	progStatus.contest = m->mvalue()->value();
}

void cb_mnuPicViewer(Fl_Menu_ *, void *) {
	if (picRxWin) {
		picRx->redraw();
		picRxWin->show();
	}
}

void cb_sldrSquelch(Fl_Slider* o, void*) {
	progStatus.sldrSquelchValue = o->value();
	restoreFocus();
}


static char ztbuf[14];

const char* zdate(void) { return ztbuf; }
const char* ztime(void) { return ztbuf + 9; }

void ztimer(void* first_call)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	double st = 60.0 - tv.tv_sec % 60 - tv.tv_usec / 1e6;
	if (!first_call) {
		tv.tv_sec = (int)(60.0 * round(tv.tv_sec / 60.0));
		if (st < 1.0)
			st += 60.0;
	}
	Fl::repeat_timeout(st, ztimer);

	struct tm tm;
	gmtime_r(&tv.tv_sec, &tm);
	if (!strftime(ztbuf, sizeof(ztbuf), "%Y%m%d %H%M", &tm))
		memset(ztbuf, 0, sizeof(ztbuf));
	else
		ztbuf[8] = '\0';

	inpTimeOff1->value(ztbuf + 9);
	inpTimeOff2->value(ztbuf + 9);
	inpTimeOff3->value(ztbuf + 9);

}


bool oktoclear = true;

void updateOutSerNo()
{
	if (contest_count.count) {
		char szcnt[10] = "";
		contest_count.Format(progdefaults.ContestDigits, progdefaults.UseLeadingZeros);
		snprintf(szcnt, sizeof(szcnt), contest_count.fmt.c_str(), contest_count.count);
		outSerNo1->value(szcnt);
		outSerNo2->value(szcnt);
	} else {
		outSerNo1->value("");
		outSerNo2->value("");
	}
}

string old_call;
string new_call;

void clearQSO()
{
if (bWF_only) return;
//jcoxon
if (bHAB) return;
//
	Fl_Input* in[] = {
		inpCall1, inpCall2, inpCall3, inpCall4,
		inpName1, inpName2,
		inpTimeOn1, inpTimeOn2, inpTimeOn3,
		inpRstIn1, inpRstIn2,
		inpRstOut1, inpRstOut2,
		inpQth, inpLoc, inpAZ, inpState, inpVEprov, inpCountry,
		inpSerNo1, inpSerNo2,
		outSerNo1, outSerNo2,
		inpXchgIn1, inpXchgIn2,
		inpNotes };
	for (size_t i = 0; i < sizeof(in)/sizeof(*in); i++)
		in[i]->value("");
	if (progdefaults.fixed599) {
		inpRstIn1->value("599"); inpRstIn2->value("599");
		inpRstOut1->value("599"); inpRstOut2->value("599");
	}
	inpCall1->color(FL_BACKGROUND2_COLOR);
	inpCall2->color(FL_BACKGROUND2_COLOR);
	inpCall3->color(FL_BACKGROUND2_COLOR);
	inpCall4->color(FL_BACKGROUND2_COLOR);
	inpCall1->redraw();
	inpCall2->redraw();
	inpCall3->redraw();
	inpCall4->redraw();
	updateOutSerNo();
	inpSearchString->value ("");
	old_call.clear();
	new_call.clear();
	qso_time.clear();
	qso_exchange.clear();
	oktoclear = true;
}

void cb_ResetSerNbr()
{
	contest_count.count = progdefaults.ContestStart;
	updateOutSerNo();
}

void cb_btnTimeOn(Fl_Widget* w, void*)
{
	inpTimeOn->value(inpTimeOff->value(), inpTimeOff->size());
	sDate_on = zdate();
	restoreFocus();
}

void cb_loc(Fl_Widget* w, void*)
{
	if ((oktoclear = !inpLoc->size()) || !progdefaults.autofill_qso_fields)
		return restoreFocus(w);

	double lon[2], lat[2], distance, azimuth;
	if (locator2longlat(&lon[0], &lat[0], progdefaults.myLocator.c_str()) == RIG_OK &&
	    locator2longlat(&lon[1], &lat[1], inpLoc->value()) == RIG_OK &&
	    qrb(lon[0], lat[0], lon[1], lat[1], &distance, &azimuth) == RIG_OK) {
		char az[4];
		snprintf(az, sizeof(az), "%3.0f", azimuth);
		inpAZ->value(az);
	}
	restoreFocus(w);
}

void cb_call(Fl_Widget* w, void*)
{
if (bWF_only) return;
//jcoxon
if (bHAB) return;
//
	if (progdefaults.calluppercase) {
		int pos = inpCall->position();
		char* uc = new char[inpCall->size()];
		transform(inpCall->value(), inpCall->value() + inpCall->size(), uc,
			  static_cast<int (*)(int)>(std::toupper));
		inpCall->value(uc, inpCall->size());
		inpCall->position(pos);
		delete [] uc;
	}

	new_call = inpCall->value();

	if (inpCall == inpCall1) {
		inpCall2->value(new_call.c_str());
		inpCall3->value(new_call.c_str());
		inpCall4->value(new_call.c_str());
	} else if (inpCall == inpCall2) {
		inpCall1->value(new_call.c_str());
		inpCall3->value(new_call.c_str());
		inpCall4->value(new_call.c_str());
	} else if (inpCall == inpCall3) {
		inpCall1->value(new_call.c_str());
		inpCall2->value(new_call.c_str());
		inpCall4->value(new_call.c_str());
	} else {
		inpCall1->value(new_call.c_str());
		inpCall2->value(new_call.c_str());
		inpCall3->value(new_call.c_str());
	}
	if (inpCall->value()[0] == 0) {
		inpCall1->color(FL_BACKGROUND2_COLOR);
		inpCall2->color(FL_BACKGROUND2_COLOR);
		inpCall3->color(FL_BACKGROUND2_COLOR);
		inpCall4->color(FL_BACKGROUND2_COLOR);
		inpCall1->redraw();
		inpCall2->redraw();
		inpCall3->redraw();
		inpCall4->redraw();
	}

	if (progStatus.timer && (Fl::event() != FL_HIDE))
		stopMacroTimer();

	if (old_call == new_call || new_call.empty())
		return restoreFocus(w);

	old_call = new_call;
	oktoclear = false;

	inpTimeOn->value(inpTimeOff->value());

	if (inpTimeOn == inpTimeOn1) inpTimeOn2->value(inpTimeOn->value());
	else inpTimeOn1->value(inpTimeOn->value());

	sDate_on = zdate();

	if (progdefaults.EnableDupCheck) {
		DupCheck();
		return restoreFocus(w);
	}

	SearchLastQSO(inpCall->value());

	if (inpAZ->value()[0])
		return restoreFocus(w);

	if (!progdefaults.autofill_qso_fields)
		return restoreFocus(w);
	const struct dxcc* e = dxcc_lookup(inpCall->value());
	if (!e)
		return restoreFocus(w);
	double lon, lat, distance, azimuth;
	if (locator2longlat(&lon, &lat, progdefaults.myLocator.c_str()) == RIG_OK &&
	    qrb(lon, lat, -e->longitude, e->latitude, &distance, &azimuth) == RIG_OK) {
		char az[4];
		snprintf(az, sizeof(az), "%3.0f", azimuth);
		inpAZ->value(az, sizeof(az) - 1);
	}
	inpCountry->value(e->country);
	inpCountry->position(0);

	restoreFocus(w);
}

void cb_log(Fl_Widget* w, void*)
{
	Fl_Input2 *inp = (Fl_Input2 *) w;

	if (inp == inpName1) inpName2->value(inpName1->value());
	if (inp == inpName2) inpName1->value(inpName2->value());
	if (inp == inpRstIn1) inpRstIn2->value(inpRstIn1->value());
	if (inp == inpRstIn2) inpRstIn1->value(inpRstIn2->value());
	if (inp == inpRstOut1) inpRstOut2->value(inpRstOut1->value());
	if (inp == inpRstOut2) inpRstOut1->value(inpRstOut2->value());

	if (inp == inpTimeOn1) {
		inpTimeOn2->value(inpTimeOn->value()); inpTimeOn3->value(inpTimeOn->value());
	}
	if (inp == inpTimeOn2) {
		inpTimeOn1->value(inpTimeOn->value()); inpTimeOn3->value(inpTimeOn->value());
	}
	if (inp == inpTimeOn3) {
		inpTimeOn1->value(inpTimeOn->value()); inpTimeOn2->value(inpTimeOn->value());
	}

	if (inp == inpTimeOff1) {
		inpTimeOff2->value(inpTimeOff->value()); inpTimeOff3->value(inpTimeOff->value());
	}
	if (inp == inpTimeOff2) {
		inpTimeOff1->value(inpTimeOff->value()); inpTimeOff3->value(inpTimeOff->value());
	}
	if (inp == inpTimeOff3) {
		inpTimeOff1->value(inpTimeOff->value()); inpTimeOff2->value(inpTimeOff->value());
	}

	if (inp == inpXchgIn1) inpXchgIn2->value(inpXchgIn1->value());
	if (inp == inpXchgIn2) inpXchgIn1->value(inpXchgIn2->value());

	if (inp->value()[0])
		oktoclear = false;
	if (progdefaults.EnableDupCheck) {
		DupCheck();
	}
	restoreFocus(w);
}

void cbClearCall(Fl_Widget *b, void *)
{
	clearQSO();
	restoreFocus();
}

void qsoClear_cb(Fl_Widget *b, void *)
{
	bool clearlog = true;
	if (progdefaults.NagMe && !oktoclear)
		clearlog = (fl_choice2(_("Clear log fields?"), _("Cancel"), _("OK"), NULL) == 1);
	if (clearlog) {
		clearQSO();
	}
	clear_Lookup();
	restoreFocus();
}

void qsoSave_cb(Fl_Widget *b, void *)
{
	string havecall = inpCall->value();
	while (!havecall.empty() && havecall[0] == ' ') havecall.erase(0,1);
	if (havecall.empty()) {
		fl_message2(_("Enter a CALL !"));
		restoreFocus();
		return;
	}
	submit_log();
	if (progdefaults.ClearOnSave)
		clearQSO();
	ReceiveText->mark(FTextBase::XMIT);
	restoreFocus();
}

void cb_QRZ(Fl_Widget *b, void *)
{
	if (!*inpCall->value())
		return restoreFocus();

	switch (Fl::event_button()) {
	case FL_LEFT_MOUSE:
		CALLSIGNquery();
		oktoclear = false;
		break;
	case FL_RIGHT_MOUSE:
		if (quick_choice(string("Spot \"").append(inpCall->value()).append("\"?").c_str(),
				 2, _("Confirm"), _("Cancel"), NULL) == 1)
			spot_manual(inpCall->value(), inpLoc->value());
		break;
	default:
		break;
	}
	restoreFocus();
}

void status_cb(Fl_Widget *b, void *arg)
{
	if (Fl::event_button() == FL_RIGHT_MOUSE) {
		progdefaults.loadDefaults();
		tabsConfigure->value(tabModems);
		tabsModems->value(modem_config_tab);
#if USE_HAMLIB
		hamlib_restore_defaults();
#endif
		rigCAT_restore_defaults();
		dlgConfig->show();
	}
	else {
		if (!quick_change)
			return;
		const Fl_Menu_Item *m = quick_change->popup(Fl::event_x(), Fl::event_y());
		if (m && m->callback())
		m->do_callback(0);
	}
	static_cast<Fl_Button*>(b)->clear();
	restoreFocus();
}

void cbAFC(Fl_Widget *w, void *vi)
{
	FL_LOCK_D();
	Fl_Button *b = (Fl_Button *)w;
	int v = b->value();
	FL_UNLOCK_D();
	progStatus.afconoff = v;
}

void cbSQL(Fl_Widget *w, void *vi)
{
	FL_LOCK_D();
	Fl_Button *b = (Fl_Button *)w;
	int v = b->value();
	FL_UNLOCK_D();
	progStatus.sqlonoff = v ? true : false;
}

void startMacroTimer()
{
	ENSURE_THREAD(FLMAIN_TID);

	btnMacroTimer->color(fl_rgb_color(240, 240, 0));
	btnMacroTimer->clear_output();
	Fl::add_timeout(0.0, macro_timer);
}

void stopMacroTimer()
{
	ENSURE_THREAD(FLMAIN_TID);

	progStatus.timer = 0;
	Fl::remove_timeout(macro_timer);

	btnMacroTimer->label(0);
	btnMacroTimer->color(FL_BACKGROUND_COLOR);
	btnMacroTimer->set_output();
}

void macro_timer(void*)
{
	char buf[8];
	snprintf(buf, sizeof(buf), "%d", progStatus.timer);
	btnMacroTimer->copy_label(buf);

	if (progStatus.timer-- == 0) {
		stopMacroTimer();
		macros.execute(progStatus.timerMacro);
	}
	else
		Fl::repeat_timeout(1.0, macro_timer);
}

void cbMacroTimerButton(Fl_Widget*, void*)
{
	stopMacroTimer();
	restoreFocus();
}

void cb_RcvMixer(Fl_Widget *w, void *d)
{
	progStatus.RcvMixer = valRcvMixer->value() / 100.0;
	mixer->setRcvGain(progStatus.RcvMixer);
}

void cb_XmtMixer(Fl_Widget *w, void *d)
{
	progStatus.XmtMixer = valXmtMixer->value() / 100.0;
	mixer->setXmtLevel(progStatus.XmtMixer);
}

int default_handler(int event)
{
	if (event != FL_SHORTCUT)
		return 0;

	if (RigViewerFrame && Fl::event_key() == FL_Escape &&
	    RigViewerFrame->visible() && Fl::event_inside(RigViewerFrame)) {
		CloseQsoView();
		return 1;
	}

	Fl_Widget* w = Fl::focus();

	if (w == fl_digi_main || w->window() == fl_digi_main) {
		int key = Fl::event_key();
		if (key == FL_Escape || (key >= FL_F && key <= FL_F_Last)) {
			TransmitText->take_focus();
			TransmitText->handle(FL_KEYBOARD);
			w->take_focus(); // remove this to leave tx text focused
			return 1;
		}
	}
	else if (w == dlgLogbook || w->window() == dlgLogbook)
		return log_search_handler(event);

	return 0;
}

bool clean_exit(void) {
	if (progdefaults.changed) {
		switch (fl_choice2(_("Save changed configuration before exiting?"),
				  _("Cancel"), _("Save"), _("Don't save"))) {
		case 0:
			return false;
		case 1:
			progdefaults.saveDefaults();
			// fall through
		case 2:
			break;
		}
	}
	if (!oktoclear && progdefaults.NagMe) {
		switch (fl_choice2(_("Save log before exiting?"),
				  _("Cancel"), _("Save"), _("Don't save"))) {
		case 0:
			return false;
		case 1:
			qsoSave_cb(0, 0);
			// fall through
		case 2:
			break;
		}
	}
	if (macros.changed) {
		switch (fl_choice2(_("Save changed macros before exiting?"),
				  _("Cancel"), _("Save"), _("Don't save"))) {
		case 0:
			return false;
		case 1:
			macros.saveMacroFile();
			// fall through
		case 2:
			break;
		}
	}
	if (Maillogfile)
		Maillogfile->log_to_file_stop();
	if (logfile)
		logfile->log_to_file_stop();

//	if (bSaveFreqList)
		saveFreqList();

	progStatus.saveLastState();

	delete push2talk;
#if USE_HAMLIB
	hamlib_close();
#endif
	rigCAT_close();
	rigMEM_close();

	if (mixer)
		mixer->closeMixer();

	if (trx_state == STATE_RX || trx_state == STATE_TX || trx_state == STATE_TUNE)
		trx_state = STATE_ABORT;
	else {
		LOG_ERROR("trx in unexpected state %d", trx_state);
		exit(1);
	}
	while (trx_state != STATE_ENDED) {
		REQ_FLUSH(GET_THREAD_ID());
		MilliSleep(10);
	}

	if (dlgConfig) {
		dlgConfig->hide();
		delete cboHamlibRig;
		delete dlgConfig;
	}

#if USE_HAMLIB
	if (xcvr) delete xcvr;
#endif

	close_logbook();

	return true;
}


#define LOG_TO_FILE_MLABEL _("Log all RX/TX text")
#define RIGCONTROL_MLABEL _("Rig control")
#define OPMODES_MLABEL _("Op &Mode")
#define OPMODES_FEWER _("Show fewer modes")
#define OPMODES_ALL _("Show all modes")
#define OLIVIA_MLABEL "Olivia"
#define CONTESTIA_MLABEL "Contestia"
#define RTTY_MLABEL "RTTY"
#define VIEW_MLABEL _("&View")
#define MFSK_IMAGE_MLABEL _("&MFSK image")
#define CONTEST_MLABEL _("Contest")
#define CONTEST_FIELDS_MLABEL _("&Contest fields")
#define COUNTRIES_MLABEL _("C&ountries")
#define UI_MLABEL _("&UI")
#define RIGLOG_FULL_MLABEL _("Full")
#define RIGLOG_NONE_MLABEL _("None")
#define RIGLOG_MLABEL      _("Rig control and logging")
#define RIGCONTEST_MLABEL  _("Rig control and contest")
#define DOCKEDSCOPE_MLABEL _("Docked scope")
#define WF_MLABEL _("Minimal controls")
// #define DLFLDIGI_ONLINE_LABEL _("Online")
const char *DLFLDIGI_ONLINE_LABEL = _("Online");

bool restore_minimize = false;

void UI_select()
{
	if (bWF_only)
		return;
//jcoxon
	if (bHAB)
		return;
//
	Fl_Menu_Item* cf = getMenuItem(CONTEST_FIELDS_MLABEL);
	if (progStatus.NO_RIGLOG || progStatus.Rig_Contest_UI || progStatus.Rig_Log_UI) {
		cf->clear();
		cf->deactivate();
	}
	else {
		cf->activate();
		if (progStatus.contest)
			cf->set();
		getMenuItem(RIGLOG_FULL_MLABEL)->setonly();
	}

	if (progStatus.NO_RIGLOG && !restore_minimize) {
		int y1 = TopFrame1->y();
		int y2 = macroFrame->y();
		int w = TopFrame1->w();
		if (MixerFrame->visible()) {
			MixerFrame->resize(0, y1, DEFAULT_SW, y2 - y1);
			MixerFrame->redraw();
			TiledGroup->resize(DEFAULT_SW, y1, w - DEFAULT_SW, y2 - y1);
			TiledGroup->redraw();
		} else {
			TiledGroup->resize(0, y1, w, y2 - y1);
			TiledGroup->redraw();
		}
		TopFrame1->hide();
		TopFrame2->hide();
		TopFrame3->hide();
		Status2->hide();
		inpCall4->show();
		inpCall = inpCall4;
		fl_digi_main->init_sizes();
		return;
	}

	if ((!progStatus.Rig_Log_UI && ! progStatus.Rig_Contest_UI) ||
			restore_minimize) {
		int y1 = TopFrame1->y() + Hqsoframe;
		int y2 = macroFrame->y();
		int w = TopFrame1->w();
		if (MixerFrame->visible()) {
			MixerFrame->resize(0, y1, DEFAULT_SW, y2 - y1);
			MixerFrame->redraw();
			TiledGroup->resize(DEFAULT_SW, y1, w - DEFAULT_SW, y2 - y1);
			TiledGroup->redraw();
		} else {
			TiledGroup->resize(0, y1, w, y2 - y1);
			TiledGroup->redraw();
		}
		inpNotes->resize(
			inpNotes->x(), inpNotes->y(),
			inpNotes->w(), 2*Hentry + pad);
		inpNotes->redraw();
		TopFrame2->hide();
		TopFrame3->hide();
		TopFrame1->show();
		inpFreq = inpFreq1;
		inpCall = inpCall1;
		inpTimeOn = inpTimeOn1;
		inpTimeOff = inpTimeOff1;
		inpName = inpName1;
		inpRstIn = inpRstIn1;
		inpRstOut = inpRstOut1;
		inpSerNo = inpSerNo1;
		outSerNo = outSerNo1;
		inpXchgIn = inpXchgIn1;
		qsoFreqDisp = qsoFreqDisp1;
		fl_digi_main->init_sizes();
	} else if (progStatus.Rig_Log_UI || progStatus.Rig_Contest_UI) {
		int y1 = TopFrame2->y() + Hentry + 3 * pad;
		int y2 = macroFrame->y();
		int w = TopFrame1->w();
		if (MixerFrame->visible()) {
			MixerFrame->resize(0, y1, DEFAULT_SW, y2 - y1);
			MixerFrame->redraw();
			TiledGroup->resize(DEFAULT_SW, y1, w - DEFAULT_SW, y2 - y1);
			TiledGroup->redraw();
		} else {
			TiledGroup->resize(0, y1, w, y2 - y1);
			TiledGroup->redraw();
		}
		if (progStatus.Rig_Log_UI) {
			TopFrame1->hide();
			TopFrame3->hide();
			TopFrame2->show();
			inpCall = inpCall2;
			inpTimeOn = inpTimeOn2;
			inpTimeOff = inpTimeOff2;
			inpName = inpName2;
			inpSerNo = inpSerNo1;
			outSerNo = outSerNo1;
			inpRstIn = inpRstIn2;
			inpRstOut = inpRstOut2;
			qsoFreqDisp = qsoFreqDisp2;
		} else if (progStatus.Rig_Contest_UI) {
			TopFrame1->hide();
			TopFrame2->hide();
			TopFrame3->show();
			inpCall = inpCall3;
			inpTimeOn = inpTimeOn3;
			inpTimeOff = inpTimeOff3;
			inpSerNo = inpSerNo2;
			outSerNo = outSerNo2;
			inpXchgIn = inpXchgIn2;
			qsoFreqDisp = qsoFreqDisp3;
		}
	}
	inpCall4->hide();
	Status2->show();
	fl_digi_main->init_sizes();
}

void cb_mnu_wf_all(Fl_Menu_* w, void *d)
{
	wf->UI_select(progStatus.WF_UI = w->mvalue()->value());
}

void cb_mnu_riglog(Fl_Menu_* w, void *d)
{
	getMenuItem(w->mvalue()->label())->setonly();
	progStatus.Rig_Log_UI = true;
	progStatus.Rig_Contest_UI = false;
	progStatus.NO_RIGLOG = false;
	UI_select();
}

void cb_mnu_rigcontest(Fl_Menu_* w, void *d)
{
	getMenuItem(w->mvalue()->label())->setonly();
	progStatus.Rig_Contest_UI = true;
	progStatus.Rig_Log_UI = false;
	progStatus.NO_RIGLOG = false;
	UI_select();
}

void cb_mnu_riglog_all(Fl_Menu_* w, void *d)
{
	getMenuItem(w->mvalue()->label())->setonly();
	progStatus.NO_RIGLOG = progStatus.Rig_Log_UI = progStatus.Rig_Contest_UI = false;
	UI_select();
}

void cb_mnu_riglog_none(Fl_Menu_* w, void *d)
{
	getMenuItem(w->mvalue()->label())->setonly();
	progStatus.NO_RIGLOG = true;
	progStatus.Rig_Log_UI = false;
	progStatus.Rig_Contest_UI = false;
	UI_select();
}

void cb_mnuDockedscope(Fl_Menu_ *w, void *d)
{
	wf->show_scope(progStatus.DOCKEDSCOPE = w->mvalue()->value());
}

void WF_UI()
{
	wf->UI_select(progStatus.WF_UI);
}

static void cb_opmode_show(Fl_Widget* w, void*);

Fl_Menu_Item menu_[] = {
{_("&File"), 0,  0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},

#if USE_SNDFILE
{ make_icon_label(_("Audio")), 0, 0, 0, FL_SUBMENU, _FL_MULTI_LABEL, 0, 14, 0},
{_("RX capture"),  0, (Fl_Callback*)cb_mnuCapture,  0, FL_MENU_TOGGLE, FL_NORMAL_LABEL, 0, 14, 0},
{_("TX generate"), 0, (Fl_Callback*)cb_mnuGenerate, 0, FL_MENU_TOGGLE, FL_NORMAL_LABEL, 0, 14, 0},
{_("Playback"),    0, (Fl_Callback*)cb_mnuPlayback, 0, FL_MENU_TOGGLE, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},
#endif

{ make_icon_label(_("Folders")), 0, 0, 0, FL_SUBMENU, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("Fldigi config..."), folder_open_icon), 0, cb_ShowConfig, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("NBEMS files..."), folder_open_icon), 0, cb_ShowNBEMS, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{ make_icon_label(_("Logs")), 0, 0, 0, FL_SUBMENU, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("New logbook")), 0, (Fl_Callback*)cb_mnuNewLogbook, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("Open logbook...")), 0, (Fl_Callback*)cb_mnuOpenLogbook, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("Save logbook")), 0, (Fl_Callback*)cb_mnuSaveLogbook, 0, FL_MENU_DIVIDER, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("Merge ADIF...")), 0, (Fl_Callback*)cb_mnuMergeADIF_log, 0, FL_MENU_DIVIDER, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("Export ADIF")), 0, (Fl_Callback*)cb_mnuExportADIF_log, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("Export Text")), 0, (Fl_Callback*)cb_mnuExportTEXT_log, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("Export CSV")), 0, (Fl_Callback*)cb_mnuExportCSV_log, 0, FL_MENU_DIVIDER, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("Cabrillo Rpt")), 0, (Fl_Callback*)cb_Export_Cabrillo, 0, FL_MENU_DIVIDER, _FL_MULTI_LABEL, 0, 14, 0},
{ LOG_TO_FILE_MLABEL, 0, cb_logfile, 0, FL_MENU_TOGGLE, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{ make_icon_label(_("Macros")), 0, 0, 0, FL_MENU_DIVIDER | FL_SUBMENU, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("Open ..."), file_open_icon), 0,  (Fl_Callback*)cb_mnuOpenMacro, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("Save ..."), save_as_icon), 0,  (Fl_Callback*)cb_mnuSaveMacro, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{ make_icon_label(_("Exit"), log_out_icon), 'x',  (Fl_Callback*)cb_E, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},
{ OPMODES_MLABEL, 0,  0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},

{ mode_info[MODE_CW].name, 0, cb_init_mode, (void *)MODE_CW, 0, FL_NORMAL_LABEL, 0, 14, 0},

{ CONTESTIA_MLABEL, 0, 0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},
{ "4/250", 0, cb_contestiaA, (void *)MODE_CONTESTIA, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ "8/250", 0, cb_contestiaB, (void *)MODE_CONTESTIA, FL_MENU_DIVIDER, FL_NORMAL_LABEL, 0, 14, 0},
{ "4/500", 0, cb_contestiaC, (void *)MODE_CONTESTIA, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ "8/500", 0, cb_contestiaD, (void *)MODE_CONTESTIA, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ "16/500", 0, cb_contestiaE, (void *)MODE_CONTESTIA, FL_MENU_DIVIDER, FL_NORMAL_LABEL, 0, 14, 0},
{ "8/1000", 0, cb_contestiaF, (void *)MODE_CONTESTIA, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ "16/1000", 0, cb_contestiaG, (void *)MODE_CONTESTIA, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ "32/1000", 0, cb_contestiaH, (void *)MODE_CONTESTIA, FL_MENU_DIVIDER, FL_NORMAL_LABEL, 0, 14, 0},
{ _("Custom..."), 0, cb_contestiaCustom, (void *)MODE_CONTESTIA, 0, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{"DominoEX", 0, 0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_DOMINOEX4].name, 0, cb_init_mode, (void *)MODE_DOMINOEX4, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_DOMINOEX5].name, 0, cb_init_mode, (void *)MODE_DOMINOEX5, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_DOMINOEX8].name, 0, cb_init_mode, (void *)MODE_DOMINOEX8, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_DOMINOEX11].name, 0, cb_init_mode, (void *)MODE_DOMINOEX11, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_DOMINOEX16].name, 0, cb_init_mode, (void *)MODE_DOMINOEX16, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_DOMINOEX22].name, 0, cb_init_mode, (void *)MODE_DOMINOEX22, 0, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{"Hell", 0, 0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_FELDHELL].name, 0, cb_init_mode, (void *)MODE_FELDHELL, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_SLOWHELL].name, 0,  cb_init_mode, (void *)MODE_SLOWHELL, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_HELLX5].name, 0,  cb_init_mode, (void *)MODE_HELLX5, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_HELLX9].name, 0,  cb_init_mode, (void *)MODE_HELLX9, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_FSKHELL].name, 0, cb_init_mode, (void *)MODE_FSKHELL, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_FSKH105].name, 0, cb_init_mode, (void *)MODE_FSKH105, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_HELL80].name, 0, cb_init_mode, (void *)MODE_HELL80, 0, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{"MFSK", 0, 0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_MFSK4].name, 0,  cb_init_mode, (void *)MODE_MFSK4, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_MFSK8].name, 0,  cb_init_mode, (void *)MODE_MFSK8, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_MFSK11].name, 0,  cb_init_mode, (void *)MODE_MFSK11, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_MFSK16].name, 0,  cb_init_mode, (void *)MODE_MFSK16, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_MFSK22].name, 0,  cb_init_mode, (void *)MODE_MFSK22, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_MFSK31].name, 0,  cb_init_mode, (void *)MODE_MFSK31, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_MFSK32].name, 0,  cb_init_mode, (void *)MODE_MFSK32, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_MFSK64].name, 0,  cb_init_mode, (void *)MODE_MFSK64, 0, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{"MT63", 0, 0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_MT63_500].name, 0,  cb_init_mode, (void *)MODE_MT63_500, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_MT63_1000].name, 0,  cb_init_mode, (void *)MODE_MT63_1000, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_MT63_2000].name, 0,  cb_init_mode, (void *)MODE_MT63_2000, 0, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{ OLIVIA_MLABEL, 0, 0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},
{ "8/250", 0, cb_oliviaA, (void *)MODE_OLIVIA, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ "8/500", 0, cb_oliviaB, (void *)MODE_OLIVIA, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ "16/500", 0, cb_oliviaC, (void *)MODE_OLIVIA, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ "8/1000", 0, cb_oliviaD, (void *)MODE_OLIVIA, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ "32/1000", 0, cb_oliviaE, (void *)MODE_OLIVIA, FL_MENU_DIVIDER, FL_NORMAL_LABEL, 0, 14, 0},
{ _("Custom..."), 0, cb_oliviaCustom, (void *)MODE_OLIVIA, 0, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{"PSK", 0, 0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_PSK31].name, 0, cb_init_mode, (void *)MODE_PSK31, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_PSK63].name, 0, cb_init_mode, (void *)MODE_PSK63, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_PSK63F].name, 0, cb_init_mode, (void *)MODE_PSK63F, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_PSK125].name, 0, cb_init_mode, (void *)MODE_PSK125, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_PSK250].name, 0, cb_init_mode, (void *)MODE_PSK250, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_PSK500].name, 0, cb_init_mode, (void *)MODE_PSK500, 0, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{"QPSK", 0, 0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_QPSK31].name, 0, cb_init_mode, (void *)MODE_QPSK31, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_QPSK63].name, 0, cb_init_mode, (void *)MODE_QPSK63, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_QPSK125].name, 0, cb_init_mode, (void *)MODE_QPSK125, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_QPSK250].name, 0, cb_init_mode, (void *)MODE_QPSK250, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_QPSK500].name, 0, cb_init_mode, (void *)MODE_QPSK500, 0, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{"PSKR", 0, 0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_PSK125R].name, 0, cb_init_mode, (void *)MODE_PSK125R, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_PSK250R].name, 0, cb_init_mode, (void *)MODE_PSK250R, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_PSK500R].name, 0, cb_init_mode, (void *)MODE_PSK500R, 0, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{ RTTY_MLABEL, 0, 0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},
{ "RTTY-45", 0, cb_rtty45, (void *)MODE_RTTY, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ "RTTY-50", 0, cb_rtty50, (void *)MODE_RTTY, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ "RTTY-75", 0, cb_rtty75, (void *)MODE_RTTY, FL_MENU_DIVIDER, FL_NORMAL_LABEL, 0, 14, 0},
{ _("Custom..."), 0, cb_rttyCustom, (void *)MODE_RTTY, 0, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{"THOR", 0, 0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_THOR4].name, 0, cb_init_mode, (void *)MODE_THOR4, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_THOR5].name, 0, cb_init_mode, (void *)MODE_THOR5, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_THOR8].name, 0, cb_init_mode, (void *)MODE_THOR8, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_THOR11].name, 0, cb_init_mode, (void *)MODE_THOR11, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_THOR16].name, 0, cb_init_mode, (void *)MODE_THOR16, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_THOR22].name, 0, cb_init_mode, (void *)MODE_THOR22, 0, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{"Throb", 0, 0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_THROB1].name, 0, cb_init_mode, (void *)MODE_THROB1, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_THROB2].name, 0, cb_init_mode, (void *)MODE_THROB2, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_THROB4].name, 0, cb_init_mode, (void *)MODE_THROB4, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_THROBX1].name, 0, cb_init_mode, (void *)MODE_THROBX1, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_THROBX2].name, 0, cb_init_mode, (void *)MODE_THROBX2, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_THROBX4].name, 0, cb_init_mode, (void *)MODE_THROBX4, 0, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{"NBEMS modes", 0, 0, 0, FL_SUBMENU | FL_MENU_DIVIDER, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_DOMINOEX11].name, 0, cb_init_mode, (void *)MODE_DOMINOEX11, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_DOMINOEX22].name, 0, cb_init_mode, (void *)MODE_DOMINOEX22, FL_MENU_DIVIDER, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_MFSK16].name, 0,  cb_init_mode, (void *)MODE_MFSK16, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_MFSK32].name, 0,  cb_init_mode, (void *)MODE_MFSK32, FL_MENU_DIVIDER, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_PSK125].name, 0, cb_init_mode, (void *)MODE_PSK125, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_PSK250].name, 0, cb_init_mode, (void *)MODE_PSK250, 0, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{ mode_info[MODE_SSB].name, 0, cb_init_mode, (void *)MODE_SSB, 0, FL_NORMAL_LABEL, 0, 14, 0},

{ mode_info[MODE_WWV].name, 0, cb_init_mode, (void *)MODE_WWV, 0, FL_NORMAL_LABEL, 0, 14, 0},

{ mode_info[MODE_ANALYSIS].name, 0, cb_init_mode, (void *)MODE_ANALYSIS, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ OPMODES_FEWER, 0, cb_opmode_show, 0, FL_MENU_INVISIBLE, FL_NORMAL_LABEL, FL_HELVETICA_ITALIC, 14, 0 },
{0,0,0,0,0,0,0,0,0},

{_("&Configure"), 0, 0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},
{ make_icon_label(_("Operator"), system_users_icon), 0, (Fl_Callback*)cb_mnuConfigOperator, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("Colors && Fonts"), preferences_desktop_font_icon), 0, (Fl_Callback*)cb_mnuConfigFonts, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("User Interface")), 0,  (Fl_Callback*)cb_mnuUI, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("Waterfall"), waterfall_icon), 0,  (Fl_Callback*)cb_mnuConfigWaterfall, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("Waterfall controls")), 0,  (Fl_Callback*)cb_mnuConfigWFcontrols, 0, FL_MENU_DIVIDER, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("Modems"), emblems_system_icon), 0, (Fl_Callback*)cb_mnuConfigModems, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(RIGCONTROL_MLABEL, multimedia_player_icon), 0, (Fl_Callback*)cb_mnuConfigRigCtrl, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("Sound Card"), audio_card_icon), 0, (Fl_Callback*)cb_mnuConfigSoundCard, 0, FL_MENU_DIVIDER, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("IDs")), 0,  (Fl_Callback*)cb_mnuConfigID, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("Misc")), 0,  (Fl_Callback*)cb_mnuConfigMisc, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("Notifications")), 0,  (Fl_Callback*)cb_mnuConfigNotify, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(CONTEST_MLABEL), 0,  (Fl_Callback*)cb_mnuConfigContest, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("QRZ"), net_icon), 0,  (Fl_Callback*)cb_mnuConfigQRZ, 0, FL_MENU_DIVIDER, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("Save Config"), save_icon), 0, (Fl_Callback*)cb_mnuSaveConfig, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{ VIEW_MLABEL, 0, 0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},
{ make_icon_label(_("Floating scope"), utilities_system_monitor_icon), 'd', (Fl_Callback*)cb_mnuDigiscope, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(MFSK_IMAGE_MLABEL, image_icon), 'm', (Fl_Callback*)cb_mnuPicViewer, 0, FL_MENU_INACTIVE, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("PSK browser")), 'p', (Fl_Callback*)cb_mnuViewer, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("Logbook")), 'l', (Fl_Callback*)cb_mnuShowLogbook, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("SSDV RX")), 's', (Fl_Callback*)cb_mnuShowSSDVRX, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(COUNTRIES_MLABEL), 'o', (Fl_Callback*)cb_mnuShowCountries, 0, FL_MENU_DIVIDER, _FL_MULTI_LABEL, 0, 14, 0},

{ make_icon_label(_("Controls")), 0, 0, 0, FL_SUBMENU, _FL_MULTI_LABEL, 0, 14, 0},
{ RIGLOG_FULL_MLABEL, 0, (Fl_Callback*)cb_mnu_riglog_all, 0, FL_MENU_RADIO, FL_NORMAL_LABEL, 0, 14, 0},
{ RIGLOG_MLABEL, 0, (Fl_Callback*)cb_mnu_riglog, 0, FL_MENU_RADIO, FL_NORMAL_LABEL, 0, 14, 0},
{ RIGCONTEST_MLABEL, 0, (Fl_Callback*)cb_mnu_rigcontest, 0, FL_MENU_RADIO, FL_NORMAL_LABEL, 0, 14, 0},
{ RIGLOG_NONE_MLABEL, 0, (Fl_Callback*)cb_mnu_riglog_none, 0, FL_MENU_RADIO, FL_NORMAL_LABEL, 0, 14, 0},
{ CONTEST_FIELDS_MLABEL, 'c', (Fl_Callback*)cb_mnuContest, 0, FL_MENU_TOGGLE, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{ make_icon_label(_("Waterfall")), 0, 0, 0, FL_SUBMENU, _FL_MULTI_LABEL, 0, 14, 0},
{ DOCKEDSCOPE_MLABEL, 0, (Fl_Callback*)cb_mnuDockedscope, 0, FL_MENU_TOGGLE, FL_NORMAL_LABEL, 0, 14, 0},
{ WF_MLABEL, 0, (Fl_Callback*)cb_mnu_wf_all, 0, FL_MENU_TOGGLE, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{0,0,0,0,0,0,0,0,0},

// {"     ", 0, 0, 0, FL_MENU_INACTIVE, FL_NORMAL_LABEL, 0, 14, 0},
{_("&Help"), 0, 0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},
#ifndef NDEBUG
// settle the gmfsk vs fldigi argument once and for all
{ make_icon_label(_("Create sunspots"), weather_clear_icon), 0, cb_mnuFun, 0, FL_MENU_DIVIDER, _FL_MULTI_LABEL, 0, 14, 0},
#endif
{ make_icon_label(_("Beginners' Guide"), start_here_icon), 0, cb_mnuBeginnersURL, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("Online documentation..."), help_browser_icon), 0, cb_mnuVisitURL, (void *)PACKAGE_DOCS, 0, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("Fldigi web site..."), net_icon), 0, cb_mnuVisitURL, (void *)PACKAGE_HOME, 0, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("Reception reports..."), pskr_icon), 0, cb_mnuVisitPSKRep, 0, FL_MENU_DIVIDER, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("Command line options"), utilities_terminal_icon), 0, cb_mnuCmdLineHelp, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("Audio device info"), audio_card_icon), 0, cb_mnuAudioInfo, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("Build info"), executable_icon), 0, cb_mnuBuildInfo, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("Event log"), dialog_information_icon), 0, cb_mnuDebug, 0, FL_MENU_DIVIDER, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("Check for updates..."), system_software_update_icon), 0, cb_mnuCheckUpdate, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("&About"), help_about_icon), 'a', cb_mnuAboutURL, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

/* TODO: Remove this. As a debug/temporary measure, I'm adding in the DL Client menu to non --hab mode,
 * since we don't yet have any other UI as complete as this to configure online/offline, etc. */
/* When you remove this; also remove the toggles entry on line TODO: 3791 */
{_("DL Client"), 0, 0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},
{ DLFLDIGI_ONLINE_LABEL, 0, cb_toggle_dl_online, 0, FL_MENU_TOGGLE, FL_NORMAL_LABEL, 0, 14, 0},
{ make_icon_label(_("Refresh Payload Data"), pskr_icon), 0, cb_dl_fldigi_refresh, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("Configure"), help_about_icon), 0, (Fl_Callback*)cb_mnuConfigDLClient, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("Tracker"), pskr_icon), 0, cb_mnuVisitTracker, 0, FL_MENU_DIVIDER, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("Raw Data"), pskr_icon), 0, cb_mnuVisitView, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("Help"), pskr_icon), 0, cb_mnuVisitDLClient, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{"  ", 0, 0, 0, FL_MENU_INACTIVE, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},
};

static int count_visible_items(Fl_Menu_Item* menu)
{
	int n = 0;
	if (menu->flags & FL_SUBMENU)
		menu++;
	while (menu->label()) {
		if (menu->visible())
			n++;
		menu++;
	}
	return n;
}

static bool modes_hidden;
static void cb_opmode_show(Fl_Widget* w, void*)
{
	Fl_Menu_* m = (Fl_Menu_*)w;
	const char* label = m->mvalue()->label();

	Fl_Menu_Item *item, *opmodes = getMenuItem(OPMODES_MLABEL);
	if (!strcmp(label, OPMODES_ALL)) {
		int n = opmodes->size();
		for (int i = 0; i < n; i++) {
			item = opmodes + i;
			if (item->label())
				item->show();
		}
		menu_[m->value()].label(OPMODES_FEWER);
		modes_hidden = false;
	}
	else {
		int n = opmodes->size() - 1;
		for (int i = 0; i < n; i++) {
			item = opmodes + i;
			if (item->label() && item->callback() == cb_init_mode) {
				intptr_t mode = (intptr_t)item->user_data();
				if (mode < NUM_RXTX_MODES) {
					if (progdefaults.visible_modes.test(mode))
						item->show();
					else
						item->hide();
				}
			}
		}
		for (int i = 0; i < n; i++) {
			item = opmodes + i;
			if (item->flags & FL_SUBMENU) {
				if (count_visible_items(item))
					item->show();
				else
					item->hide();
			}
		}
		if (progdefaults.visible_modes.test(MODE_OLIVIA))
			getMenuItem("Olivia")->show();
		else
			getMenuItem("Olivia")->hide();
		if (progdefaults.visible_modes.test(MODE_CONTESTIA))
			getMenuItem("Contestia")->show();
		else
			getMenuItem("Contestia")->hide();
		if (progdefaults.visible_modes.test(MODE_RTTY))
			getMenuItem("RTTY")->show();
		else
			getMenuItem("RTTY")->hide();
		menu_[m->value()].label(OPMODES_ALL);
		modes_hidden = true;
	}
	m->redraw();
}

void toggle_visible_modes(Fl_Widget*, void*)
{
	Fl_Menu_Item* show_modes = modes_hidden ? getMenuItem(OPMODES_ALL) : getMenuItem(OPMODES_FEWER);

	if (!(~progdefaults.visible_modes).none()) { // some modes disabled
		show_modes->label(OPMODES_FEWER);
		show_modes->show();
		(show_modes - 1)->flags |= FL_MENU_DIVIDER;
		mnu->value(show_modes);
		show_modes->do_callback(mnu, (void*)0);
	}
	else {
		mnu->value(show_modes);
		show_modes->do_callback(mnu, (void*)0);
		show_modes->hide();
		(show_modes - 1)->flags &= ~FL_MENU_DIVIDER;
	}
}

Fl_Menu_Item *getMenuItem(const char *caption, Fl_Menu_Item* submenu)
{
	if (submenu == 0 || !(submenu->flags & FL_SUBMENU))
		submenu = menu_;

	int size = submenu->size() - 1;
	Fl_Menu_Item *item = 0;
	const char* label;
	for (int i = 0; i < size; i++) {
		label = (submenu[i].labeltype() == _FL_MULTI_LABEL) ?
			get_icon_label_text(&submenu[i]) : submenu[i].text;
		if (label && !strcmp(label, caption)) {
			item = submenu + i;
			break;
		}
	}
	if (!item)
		LOG_ERROR("FIXME: could not find menu \"%s\"", caption);
	return item;
}

void activate_mfsk_image_item(bool b)
{
	Fl_Menu_Item *mfsk_item = getMenuItem(MFSK_IMAGE_MLABEL);
	if (mfsk_item)
		set_active(mfsk_item, b);
}

int rightof(Fl_Widget* w)
{
	int a = w->align();

	fl_font(FL_HELVETICA, FL_NORMAL_SIZE);
	int lw = static_cast<int>(ceil(fl_width(w->label())));

	if (a & FL_ALIGN_INSIDE)
		return w->x() + w->w();

	if (a & (FL_ALIGN_TOP | FL_ALIGN_BOTTOM)) {
		if (a & FL_ALIGN_LEFT)
			return w->x() + MAX(w->w(), lw);
		else if (a & FL_ALIGN_RIGHT)
			return  w->x() + w->w();
		else
			return  w->x() + ((lw > w->w()) ? (lw - w->w())/2 : w->w());
	} else
		return w->x() + w->w() + lw;
}

int leftof(Fl_Widget* w)
{
	int a = w->align();
	if (a == FL_ALIGN_CENTER || a & FL_ALIGN_INSIDE)
		return w->x();

	fl_font(FL_HELVETICA, FL_NORMAL_SIZE);
	int lw = static_cast<int>(ceil(fl_width(w->label())));

	if (a & (FL_ALIGN_TOP | FL_ALIGN_BOTTOM)) {
		if (a & FL_ALIGN_LEFT)
			return w->x();
		else if (a & FL_ALIGN_RIGHT)
			return w->x() - (lw > w->w() ? lw - w->w() : 0);
		else
			return w->x() - (lw > w->w() ? (lw - w->w())/2 : 0);
	}
	else {
		if (a & FL_ALIGN_LEFT)
			return w->x() - lw;
		else
			return w->x();
	}
}

int above(Fl_Widget* w)
{
	int a = w->align();
	if (a == FL_ALIGN_CENTER || a & FL_ALIGN_INSIDE)
		return w->y();

	return (a & FL_ALIGN_TOP) ? w->y() + FL_NORMAL_SIZE : w->y();
}

int below(Fl_Widget* w)
{
	int a = w->align();
	if (a == FL_ALIGN_CENTER || a & FL_ALIGN_INSIDE)
		return w->y() + w->h();

	return (a & FL_ALIGN_BOTTOM) ? w->y() + w->h() + FL_NORMAL_SIZE : w->y() + w->h();
}

string main_window_title;
void update_main_title()
{
	string buf = main_window_title;
	buf.append(" - ");
	if (bWF_only)
		buf.append(_("waterfall-only mode"));
//jcoxon
	else if (bHAB)
		buf = "dl-fldigi for High Altitude Balloon Tracking";
//
	else
		buf.append(progdefaults.myCall.empty() ? _("NO CALLSIGN SET") : progdefaults.myCall.c_str());
	if (fl_digi_main)
		fl_digi_main->copy_label(buf.c_str());
}

void showOpBrowserView(Fl_Widget *, void *)
{
	if (RigViewerFrame->visible())
		return CloseQsoView();
	QsoInfoFrame1->hide();
	QsoInfoFrame2->hide();
	QsoButtonFrame->hide();
	RigViewerFrame->show();
	qso_opPICK->box(FL_DOWN_BOX);
	qso_opBrowser->take_focus();
	qso_opPICK->tooltip(_("Close List"));
}

void CloseQsoView()
{
	RigViewerFrame->hide();
	QsoInfoFrame1->show();
	QsoInfoFrame2->show();
	QsoButtonFrame->show();
	qso_opPICK->box(FL_UP_BOX);
	qso_opPICK->tooltip(_("Open List"));
	if (restore_minimize) {
		restore_minimize = false;
		UI_select();
	}
}

void showOpBrowserView2(Fl_Widget *w, void *)
{
	restore_minimize = true;
	UI_select();
	showOpBrowserView(w, NULL);
}

void cb_qso_btnSelFreq(Fl_Widget *, void *)
{
	qso_selectFreq();
}

void cb_qso_btnDelFreq(Fl_Widget *, void *)
{
	qso_delFreq();
}

void cb_qso_btnAddFreq(Fl_Widget *, void *)
{
	qso_addFreq();
}

void cb_qso_btnClearList(Fl_Widget *, void *)
{
	if (quick_choice(_("Clear list?"), 2, _("Confirm"), _("Cancel"), NULL) == 1)
		clearList();
}

void cb_qso_inpAct(Fl_Widget*, void*)
{
	string data, url;
	data.reserve(128);
	url = "http://pskreporter.info/cgi-bin/psk-freq.pl";
	if (qso_inpAct->size())
		url.append("?grid=").append(qso_inpAct->value());
	else if (progdefaults.myLocator.length() > 2)
		url.append("?grid=").append(progdefaults.myLocator, 0, 2);

	string::size_type i;
	if (!fetch_http_gui(url, data, 10.0, busy_cursor, 0, default_cursor, 0) ||
	    (i = data.find("\r\n\r\n")) == string::npos) {
		LOG_ERROR("Error while fetching \"%s\": %s", url.c_str(), data.c_str());
		return;
	}

	i += strlen("\r\n\r\n");
	re_t re("([[:digit:]]{6,}) [[:digit:]]+ ([[:digit:]]+)[[:space:]]+", REG_EXTENDED);

	const size_t menu_max = 8;
	Fl_Menu_Item menu[menu_max + 1];
	string str[menu_max];
	size_t j = 0;
	memset(menu, 0, sizeof(menu));

	while (re.match(data.c_str() + i) && j < menu_max) {
		i += re.submatch(0).length();
		str[j].assign(re.submatch(1)).append(" (").append(re.submatch(2)).
			append(" ").append(atoi(re.submatch(2).c_str()) == 1 ? _("report") : _("reports")).append(")");
		menu[j].label(str[j].c_str());
		menu[++j].label(NULL);
	}

	if ((i = data.find(" grid ", i)) != string::npos)
		data.assign(data, i + strlen(" grid"), 3);
	else
		data = " (?)";
	if (j)
		data.insert(0, _("Recent activity for grid"));
	else
		data = "No recent activity";

	if ((j = quick_choice_menu(data.c_str(), 1, menu)))
		qsy(strtoll(str[j - 1].erase(str[j - 1].find(' ')).c_str(), NULL, 10));
}

void cb_qso_opBrowser(Fl_Browser*, void*)
{
	int i = qso_opBrowser->value();
	if (!i)
		return;

	// This makes the multi browser behave more like a hold browser,
	// but with the ability to invoke the callback via space/return.
	qso_opBrowser->deselect();
	qso_opBrowser->select(i);

	switch (i = Fl::event_key()) {
	case FL_Enter: case FL_KP_Enter: case FL_Button + FL_LEFT_MOUSE:
		if (i == FL_Button + FL_LEFT_MOUSE && !Fl::event_clicks())
			break;
		qso_selectFreq();
		CloseQsoView();
		break;
	case ' ': case FL_Button + FL_RIGHT_MOUSE:
		qso_setFreq();
		break;
	case FL_Button + FL_MIDDLE_MOUSE:
		i = qso_opBrowser->value();
		qso_delFreq();
		qso_addFreq();
		qso_opBrowser->select(i);
		break;
	}
}

void show_frequency(long long freq)
{
	qsoFreqDisp1->value(freq);
	qsoFreqDisp2->value(freq);
	qsoFreqDisp3->value(freq);
}

void show_mode(const string& sMode)
{
	REQ_SYNC(&Fl_ComboBox::put_value, qso_opMODE, sMode.c_str());
}

void show_bw(const string& sWidth)
{
	REQ_SYNC(&Fl_ComboBox::put_value, qso_opBW, sWidth.c_str());
}


void show_spot(bool v)
{
//if (bWF_only) return;
	static bool oldval = false;
	if (v) {
		mnu->size(btnAutoSpot->x(), mnu->h());
		if (oldval)
			progStatus.spot_recv = true;
		btnAutoSpot->value(progStatus.spot_recv);
		btnAutoSpot->activate();
	}
	else {
		btnAutoSpot->deactivate();
		oldval = btnAutoSpot->value();
		btnAutoSpot->value(v);
		btnAutoSpot->do_callback();
		mnu->size(btnRSID->x(), mnu->h());
	}
	mnu->redraw();
}

void setTabColors()
{
	tabsColors->selection_color(progdefaults.TabsColor);
	tabsConfigure->selection_color(progdefaults.TabsColor);
	tabsUI->selection_color(progdefaults.TabsColor);
	tabsWaterfall->selection_color(progdefaults.TabsColor);
	tabsModems->selection_color(progdefaults.TabsColor);
	tabsCW->selection_color(progdefaults.TabsColor);
	tabsPSK->selection_color(progdefaults.TabsColor);
	tabsRig->selection_color(progdefaults.TabsColor);
	tabsSoundCard->selection_color(progdefaults.TabsColor);
	tabsMisc->selection_color(progdefaults.TabsColor);
	if (dlgConfig->visible()) dlgConfig->redraw();
	if (dlgColorFont->visible()) dlgColorFont->redraw();
}

void showMacroSet() {
	if (progdefaults.DisplayMacroFilename) {
		string Macroset = "\n<<<===== Macro File ";
		Macroset.append(progStatus.LastMacroFile);
		Macroset.append(" Loaded =====>>>\n");
		ReceiveText->add(Macroset.c_str());
	}
}

void setwfrange() {
	wf->opmode();
}

void sync_cw_parameters()
{
	active_modem->sync_parameters();
	active_modem->update_Status();
}

void cb_cntCW_WPM(Fl_Widget * w, void *v)
{
	Fl_Counter2 *cnt = (Fl_Counter2 *) w;
	progdefaults.CWspeed = (int)cnt->value();
	sldrCWxmtWPM->value(progdefaults.CWspeed);
	if (sldrCWfarnsworth->value() > progdefaults.CWspeed)
		sldrCWfarnsworth->value(progdefaults.CWspeed);
	sldrCWfarnsworth->maximum(progdefaults.CWspeed);
	progdefaults.changed = true;
	sync_cw_parameters();
	restoreFocus();
}

void cb_btnCW_Default(Fl_Widget *w, void *v)
{
	active_modem->toggleWPM();
	restoreFocus();
}


void create_fl_digi_main_primary() {

	int fnt = fl_font();
	int fsize = fl_size();
	int freqheight = Hentry + 2 * pad;
	fl_font(fnt, freqheight);
	int freqwidth = (int)fl_width("999999999") + 10;
	fl_font(fnt, fsize);
	int rig_control_width = freqwidth + 2 * pad;

	int Y = 0;

	x_qsoframe += rig_control_width;

	IMAGE_WIDTH = 4000;

	Hwfall = progdefaults.wfheight;

	Wwfall = progStatus.mainW - 2 * DEFAULT_SW;

	fl_digi_main = new Fl_Double_Window(progStatus.mainW, progStatus.mainH);

		mnuFrame = new Fl_Group(0,0,progStatus.mainW, Hmenu);
			mnu = new Fl_Menu_Bar(0, 0, progStatus.mainW - 250 - pad, Hmenu);
			// do some more work on the menu
			for (size_t i = 0; i < sizeof(menu_)/sizeof(menu_[0]); i++) {
				// FL_NORMAL_SIZE may have changed; update the menu items
				if (menu_[i].text) {
					menu_[i].labelsize_ = FL_NORMAL_SIZE;
				}
				// set the icon label for items with the multi label type
				if (menu_[i].labeltype() == _FL_MULTI_LABEL)
					set_icon_label(&menu_[i]);
			}
			mnu->menu(menu_);
			toggle_visible_modes(NULL, NULL);

			btnAutoSpot = new Fl_Light_Button(progStatus.mainW - 250 - pad, 0, 50, Hmenu, "Spot");
			btnAutoSpot->selection_color(progdefaults.SpotColor);
			btnAutoSpot->callback(cbAutoSpot, 0);
			btnAutoSpot->deactivate();

			btnRSID = new Fl_Light_Button(progStatus.mainW - 200 - pad, 0, 50, Hmenu, "RxID");
			btnRSID->selection_color(progdefaults.RxIDColor);
			btnRSID->tooltip("Receive RSID");
			btnRSID->callback(cbRSID, 0);

			btnTxRSID = new Fl_Light_Button(progStatus.mainW - 150 - pad, 0, 50, Hmenu, "TxID");
			btnTxRSID->selection_color(progdefaults.TxIDColor);
			btnTxRSID->tooltip("Transmit RSID");
			btnTxRSID->callback(cbTxRSID, 0);

			btnTune = new Fl_Light_Button(progStatus.mainW - 100 - pad, 0, 50, Hmenu, "TUNE");
			btnTune->selection_color(progdefaults.TuneColor);
			btnTune->callback(cbTune, 0);

			btnMacroTimer = new Fl_Button(progStatus.mainW - 50 - pad, 0, 50, Hmenu);
			btnMacroTimer->labelcolor(FL_DARK_RED);
			btnMacroTimer->callback(cbMacroTimerButton);
			btnMacroTimer->set_output();

			mnuFrame->resizable(mnu);
		mnuFrame->end();

		// reset the message dialog font
		fl_message_font(FL_HELVETICA, FL_NORMAL_SIZE);

		// reset the tooltip font
		Fl_Tooltip::font(FL_HELVETICA);
		Fl_Tooltip::size(FL_NORMAL_SIZE);
		Fl_Tooltip::enable(progdefaults.tooltips);

		TopFrame1 = new Fl_Group(0, Hmenu, progStatus.mainW, Hqsoframe);

		RigControlFrame = new Fl_Group(
			0, Hmenu,
			rig_control_width, Hqsoframe);

			txtRigName = new Fl_Box(pad, Hmenu, freqwidth, Hentry);
			txtRigName->align(FL_ALIGN_CENTER);
			txtRigName->color(FL_BACKGROUND_COLOR);
			txtRigName->label(_("No rig specified"));

			qsoFreqDisp1 = new cFreqControl(
				pad, Hmenu + Hentry,
				freqwidth, freqheight, "");

			qsoFreqDisp1->box(FL_DOWN_BOX);
			qsoFreqDisp1->color(FL_BACKGROUND_COLOR);
			qsoFreqDisp1->selection_color(FL_BACKGROUND_COLOR);
			qsoFreqDisp1->labeltype(FL_NORMAL_LABEL);
			qsoFreqDisp1->font(progdefaults.FreqControlFontnbr);
			qsoFreqDisp1->labelsize(12);
			qsoFreqDisp1->labelcolor(FL_FOREGROUND_COLOR);
			qsoFreqDisp1->align(FL_ALIGN_CENTER);
			qsoFreqDisp1->when(FL_WHEN_RELEASE);
			qsoFreqDisp1->callback(qso_movFreq);
			qsoFreqDisp1->SetONOFFCOLOR(
				fl_rgb_color(	progdefaults.FDforeground.R,
								progdefaults.FDforeground.G,
								progdefaults.FDforeground.B),
				fl_rgb_color(	progdefaults.FDbackground.R,
								progdefaults.FDbackground.G,
								progdefaults.FDbackground.B));
			qsoFreqDisp1->value(0);

			Y = Hmenu + 2 * (Hentry + pad);

				int w_pmb = (freqwidth - Wbtn + 2 * pad) / 2;

				qso_opMODE = new Fl_ComboBox(
					pad, Hmenu + 2 * (Hentry + pad) + pad,
					w_pmb, Hentry);
				qso_opMODE->box(FL_DOWN_BOX);
				qso_opMODE->color(FL_BACKGROUND2_COLOR);
				qso_opMODE->selection_color(FL_BACKGROUND_COLOR);
				qso_opMODE->labeltype(FL_NORMAL_LABEL);
				qso_opMODE->labelfont(0);
				qso_opMODE->labelsize(14);
				qso_opMODE->labelcolor(FL_FOREGROUND_COLOR);
				qso_opMODE->callback((Fl_Callback*)cb_qso_opMODE);
				qso_opMODE->align(FL_ALIGN_TOP);
				qso_opMODE->when(FL_WHEN_RELEASE);
				qso_opMODE->end();

				qso_opBW = new Fl_ComboBox(
					rightof(qso_opMODE), Hmenu + 2 * (Hentry + pad) + pad,
					w_pmb, Hentry);
				qso_opBW->box(FL_DOWN_BOX);
				qso_opBW->color(FL_BACKGROUND2_COLOR);
				qso_opBW->selection_color(FL_BACKGROUND_COLOR);
				qso_opBW->labeltype(FL_NORMAL_LABEL);
				qso_opBW->labelfont(0);
				qso_opBW->labelsize(14);
				qso_opBW->labelcolor(FL_FOREGROUND_COLOR);
				qso_opBW->callback((Fl_Callback*)cb_qso_opBW);
				qso_opBW->align(FL_ALIGN_TOP);
				qso_opBW->when(FL_WHEN_RELEASE);
				qso_opBW->end();

				qso_opPICK = new Fl_Button(
					rightof(qso_opBW), Hmenu + 2 * (Hentry + pad) + pad,
					Wbtn, Hentry);
				addrbookpixmap = new Fl_Pixmap(address_book_icon);
	 			qso_opPICK->image(addrbookpixmap);
				qso_opPICK->callback(showOpBrowserView, 0);
				qso_opPICK->tooltip(_("Open List"));
		RigControlFrame->resizable(NULL);

		RigControlFrame->end();

		int opB_w = 280;
		int qFV_w = opB_w + 2 * (Wbtn + pad) + pad;

		RigViewerFrame = new Fl_Group(rightof(RigControlFrame), Hmenu, qFV_w, Hqsoframe);

			qso_btnSelFreq = new Fl_Button(
				rightof(RigControlFrame), Hmenu + pad,
				Wbtn, Hentry);
			qso_btnSelFreq->image(new Fl_Pixmap(left_arrow_icon));
			qso_btnSelFreq->tooltip(_("Select"));
			qso_btnSelFreq->callback((Fl_Callback*)cb_qso_btnSelFreq);

			qso_btnAddFreq = new Fl_Button(
				rightof(qso_btnSelFreq) + pad, Hmenu + pad,
				Wbtn, Hentry);
			qso_btnAddFreq->image(new Fl_Pixmap(plus_icon));
			qso_btnAddFreq->tooltip(_("Add current frequency"));
			qso_btnAddFreq->callback((Fl_Callback*)cb_qso_btnAddFreq);

			qso_btnClearList = new Fl_Button(
				rightof(RigControlFrame), Hmenu + Hentry + 2 * pad,
				Wbtn, Hentry);
			qso_btnClearList->image(new Fl_Pixmap(trash_icon));
			qso_btnClearList->tooltip(_("Clear list"));
			qso_btnClearList->callback((Fl_Callback*)cb_qso_btnClearList);

			qso_btnDelFreq = new Fl_Button(
				rightof(qso_btnClearList) + pad, Hmenu + Hentry + 2 * pad,
				Wbtn, Hentry);
			qso_btnDelFreq->image(new Fl_Pixmap(minus_icon));
			qso_btnDelFreq->tooltip(_("Delete from list"));
			qso_btnDelFreq->callback((Fl_Callback*)cb_qso_btnDelFreq);

			qso_btnAct = new Fl_Button(
				rightof(RigControlFrame), Hmenu + 2*(Hentry + pad) + pad,
				Wbtn, Hentry);
			qso_btnAct->image(new Fl_Pixmap(chat_icon));
			qso_btnAct->callback(cb_qso_inpAct);
			qso_btnAct->tooltip("Show active frequencies");

			qso_inpAct = new Fl_Input2(
				rightof(qso_btnAct) + pad, Hmenu + 2*(Hentry + pad) + pad,
				Wbtn, Hentry);
			qso_inpAct->when(FL_WHEN_ENTER_KEY | FL_WHEN_NOT_CHANGED);
			qso_inpAct->callback(cb_qso_inpAct);
			qso_inpAct->tooltip("Grid prefix for activity list");

			qso_opBrowser = new Fl_Browser(
				rightof(qso_btnDelFreq) + pad,  Hmenu + pad,
				opB_w, Hqsoframe - 2 * pad );
			qso_opBrowser->tooltip(_("Select operating parameters"));
			qso_opBrowser->callback((Fl_Callback*)cb_qso_opBrowser);
			qso_opBrowser->type(FL_MULTI_BROWSER);
			qso_opBrowser->box(FL_DOWN_BOX);
			qso_opBrowser->labelfont(4);
			qso_opBrowser->labelsize(12);
			qso_opBrowser->textfont(4);
			RigViewerFrame->resizable(NULL);

		RigViewerFrame->end();
		RigViewerFrame->hide();

		QsoButtonFrame = new Fl_Group(rightof(
				RigControlFrame), Hmenu,
				Wbtn, Hqsoframe);
			btnQRZ = new Fl_Button(
					rightof(RigControlFrame), Hmenu + pad,
					Wbtn, Hentry);
			btnQRZ->image(new Fl_Pixmap(net_icon));
			btnQRZ->callback(cb_QRZ, 0);
			btnQRZ->tooltip(_("QRZ"));

			qsoClear = new Fl_Button(
					rightof(RigControlFrame), Hmenu + 2 * pad + Hentry,
					Wbtn, Hentry);
			qsoClear->image(new Fl_Pixmap(edit_clear_icon));
			qsoClear->callback(qsoClear_cb, 0);
			qsoClear->tooltip(_("Clear"));

			qsoSave = new Fl_Button(
					rightof(RigControlFrame), Hmenu + 2*(pad + Hentry) + pad,
					Wbtn, Hentry);
			qsoSave->image(new Fl_Pixmap(save_icon));
			qsoSave->callback(qsoSave_cb, 0);
			qsoSave->tooltip(_("Save"));
		QsoButtonFrame->end();
		QsoButtonFrame->resizable(NULL);

		int y2 = Hmenu + Hentry + 2 * pad;
		int y3 = Hmenu + 2 * (Hentry + pad) + pad;

		QsoInfoFrame = new Fl_Group(x_qsoframe, Hmenu,
						progStatus.mainW - rightof(QsoButtonFrame) - pad, Hqsoframe);

			QsoInfoFrame1 = new Fl_Group(x_qsoframe, Hmenu, wf1, Hqsoframe);

				inpFreq1 = new Fl_Input2(x_qsoframe + pad, y2, w_inpFreq, Hentry, _("QSO Freq"));
				inpFreq1->type(FL_NORMAL_OUTPUT);
				inpFreq1->tooltip("");
				inpFreq1->align(FL_ALIGN_TOP | FL_ALIGN_LEFT);

				inpTimeOn1 = new Fl_Input2(rightof(inpFreq1) + pad, y2, w_inpTime, Hentry, "");
				inpTimeOn1->tooltip("");
				inpTimeOn1->align(FL_ALIGN_TOP | FL_ALIGN_LEFT);
				inpTimeOn1->type(FL_INT_INPUT);

				btnTimeOn = new Fl_Button(leftof(inpTimeOn1), Hmenu + pad, w_inpTime, Hentry, _("On"));
				btnTimeOn->align(FL_ALIGN_LEFT | FL_ALIGN_BOTTOM | FL_ALIGN_INSIDE);
				btnTimeOn->tooltip(_("Press to update"));
				btnTimeOn->box(FL_NO_BOX);
				btnTimeOn->callback(cb_btnTimeOn);

				inpTimeOff1 = new Fl_Input2(rightof(inpTimeOn1) + pad, y2, w_inpTime, Hentry, _("Off"));
				inpTimeOff1->tooltip("");
				inpTimeOff1->align(FL_ALIGN_TOP | FL_ALIGN_LEFT);
				inpTimeOff1->type(FL_NORMAL_OUTPUT);

				inpCall1 = new Fl_Input2(rightof(inpTimeOff1) + pad, y2, w_inpCall, Hentry, _("Call"));
				inpCall1->tooltip("");
				inpCall1->align(FL_ALIGN_TOP | FL_ALIGN_LEFT);

				inpName1 = new Fl_Input2(rightof(inpCall1) + pad, y2, w_inpName, Hentry, _("Name"));
				inpName1->tooltip("");
				inpName1->align(FL_ALIGN_TOP | FL_ALIGN_LEFT);

				inpRstIn1 = new Fl_Input2(rightof(inpName1) + pad, y2, w_inpRstIn, Hentry, _("In"));
				inpRstIn1->tooltip("");
				inpRstIn1->align(FL_ALIGN_TOP | FL_ALIGN_LEFT);

				inpRstOut1 = new Fl_Input2(rightof(inpRstIn1) + pad, y2, w_inpRstOut, Hentry, _("Out"));
				inpRstOut1->tooltip("");
				inpRstOut1->align(FL_ALIGN_TOP | FL_ALIGN_LEFT);

				QsoInfoFrame1A = new Fl_Group (x_qsoframe, y3, wf1, Hentry + pad);
					Fl_Box *fm1box = new Fl_Box(x_qsoframe, y3, w_fm1, Hentry, _("QTH"));
					fm1box->align(FL_ALIGN_INSIDE);
					inpQth = new Fl_Input2( rightof(fm1box), y3, w_inpQth, Hentry, "");
					inpQth->tooltip(_("City"));
					inpQth->align(FL_ALIGN_INSIDE);

					Fl_Box *fm2box = new Fl_Box(rightof(inpQth), y3, w_fm2, Hentry, _("St"));
					fm2box->align(FL_ALIGN_INSIDE);
					inpState = new Fl_Input2(rightof(fm2box), y3, w_inpState, Hentry, "");
					inpState->tooltip(_("US State"));
					inpState->align(FL_ALIGN_INSIDE);

					Fl_Box *fm3box = new Fl_Box(rightof(inpState), y3, w_fm3, Hentry, _("Pr"));
					fm3box->align(FL_ALIGN_INSIDE);
					inpVEprov = new Fl_Input2(rightof(fm3box), y3, w_inpProv, Hentry, "");
					inpVEprov->tooltip(_("Can. Province"));
					inpVEprov->align(FL_ALIGN_INSIDE);

					Fl_Box *fm11box = new Fl_Box(rightof(inpVEprov), y3, w_fm6, Hentry, _("Cnty"));
					fm11box->align(FL_ALIGN_INSIDE);
					inpCountry = new Fl_Input2(rightof(fm11box), y3, w_inpCountry, Hentry, "");
					inpCountry->tooltip(_("Country"));
					inpCountry->align(FL_ALIGN_INSIDE);

					Fl_Box *fm4box = new Fl_Box(rightof(inpCountry), y3, w_fm4, Hentry, _("Loc"));
					fm4box->align(FL_ALIGN_INSIDE);
					inpLoc = new Fl_Input2(rightof(fm4box), y3, w_inpLOC, Hentry, "");
					inpLoc->tooltip("");
					inpLoc->align(FL_ALIGN_INSIDE);

					Fl_Box *fm5box = new Fl_Box(rightof(inpLoc), y3, w_fm5, Hentry, _("Az"));
					fm5box->align(FL_ALIGN_INSIDE);
					inpAZ = new Fl_Input2(rightof(fm5box), y3,
					    rightof(inpRstOut1) - rightof(fm5box), Hentry, "");
					inpAZ->tooltip("");
					inpAZ->align(FL_ALIGN_INSIDE);

				QsoInfoFrame1A->end();

				QsoInfoFrame1B = new Fl_Group (
						x_qsoframe, y3,
						wf1, Hentry + pad);
					Fl_Box *fm6box = new Fl_Box(
						x_qsoframe, y3,
						w_fm7, Hentry, _("#Out"));
					fm6box->align(FL_ALIGN_INSIDE);
					outSerNo1 = new Fl_Input2(
						rightof(fm6box), y3,
						w_SerNo, Hentry, "");
					outSerNo1->align(FL_ALIGN_TOP | FL_ALIGN_LEFT);
					outSerNo1->tooltip(_("Sent serial number (read only)"));
					outSerNo1->type(FL_NORMAL_OUTPUT);

					Fl_Box *fm7box = new Fl_Box(
						rightof(outSerNo1) + pad, y3,
						w_fm5, Hentry, _("#In"));
					fm7box->align(FL_ALIGN_INSIDE);
					inpSerNo1 = new Fl_Input2(
						rightof(fm7box), y3,
						w_SerNo, Hentry, "");
					inpSerNo1->align(FL_ALIGN_TOP | FL_ALIGN_LEFT);
					inpSerNo1->tooltip(_("Received serial number"));

					Fl_Box *fm8box = new Fl_Box(
						rightof(inpSerNo1) + pad, y3,
						w_fm7, Hentry, _("Xchg"));
					fm8box->align(FL_ALIGN_INSIDE);
					inpXchgIn1 = new Fl_Input2(
						rightof(fm8box), y3,
					    rightof(inpRstOut1) - rightof(fm8box), Hentry, "");
					inpXchgIn1->align(FL_ALIGN_TOP | FL_ALIGN_LEFT);
					inpXchgIn1->tooltip(_("Contest exchange in"));

				QsoInfoFrame1B->end();
				QsoInfoFrame1B->hide();

				QsoInfoFrame1->resizable(NULL);
			QsoInfoFrame1->end();

			QsoInfoFrame2 = new Fl_Group(
				x_qsoframe + wf1 + pad, Hmenu,
				progStatus.mainW - rightof(QsoInfoFrame1) - 2*pad, Hqsoframe);

				inpNotes = new Fl_Input2(
					x_qsoframe + wf1 + pad, y2,
					progStatus.mainW - rightof(QsoInfoFrame1) - pad, 2*Hentry + pad, _("Notes"));
				inpNotes->type(FL_MULTILINE_INPUT);
				inpNotes->align(FL_ALIGN_TOP | FL_ALIGN_LEFT);

				Fl_Group::current()->resizable(inpNotes);
			QsoInfoFrame2->end();
			Fl_Group::current()->resizable(QsoInfoFrame2);
		QsoInfoFrame->end();

		Fl_Group::current()->resizable(QsoInfoFrame);

		TopFrame1->end();

		TopFrame2 = new Fl_Group(0, Hmenu, progStatus.mainW, Hentry + 2 * pad);
		{
			int y = Hmenu + pad;
			int h = Hentry;
			qsoFreqDisp2 = new cFreqControl(
				pad, Hmenu,
				freqwidth, freqheight, "");
			qsoFreqDisp2->box(FL_DOWN_BOX);
			qsoFreqDisp2->color(FL_BACKGROUND_COLOR);
			qsoFreqDisp2->selection_color(FL_BACKGROUND_COLOR);
			qsoFreqDisp2->labeltype(FL_NORMAL_LABEL);
			qsoFreqDisp2->align(FL_ALIGN_CENTER);
			qsoFreqDisp2->when(FL_WHEN_RELEASE);
			qsoFreqDisp2->callback(qso_movFreq);
			qsoFreqDisp2->font(progdefaults.FreqControlFontnbr);
			qsoFreqDisp2->SetONOFFCOLOR(
				fl_rgb_color(	progdefaults.FDforeground.R,
								progdefaults.FDforeground.G,
								progdefaults.FDforeground.B),
				fl_rgb_color(	progdefaults.FDbackground.R,
								progdefaults.FDbackground.G,
								progdefaults.FDbackground.B));
			qsoFreqDisp2->value(0);

			qso_opPICK2 = new Fl_Button(
				rightof(qsoFreqDisp2), y,
				Wbtn, Hentry);
			qso_opPICK2->image(addrbookpixmap);
			qso_opPICK2->callback(showOpBrowserView2, 0);
			qso_opPICK2->tooltip(_("Open List"));

			btnQRZ2 = new Fl_Button(
					pad + rightof(qso_opPICK2), y,
					Wbtn, Hentry);
			btnQRZ2->image(new Fl_Pixmap(net_icon));
			btnQRZ2->callback(cb_QRZ, 0);
			btnQRZ2->tooltip(_("QRZ"));

			qsoClear2 = new Fl_Button(
					pad + rightof(btnQRZ2), y,
					Wbtn, Hentry);
			qsoClear2->image(new Fl_Pixmap(edit_clear_icon));
			qsoClear2->callback(qsoClear_cb, 0);
			qsoClear2->tooltip(_("Clear"));

			qsoSave2 = new Fl_Button(
					pad + rightof(qsoClear2), y,
					Wbtn, Hentry);
			qsoSave2->image(new Fl_Pixmap(save_icon));
			qsoSave2->callback(qsoSave_cb, 0);
			qsoSave2->tooltip(_("Save"));

			const char *label2 = _("On");
			btnTimeOn2 = new Fl_Button(
				pad + rightof(qsoSave2), y,
				static_cast<int>(fl_width(label2)), h, label2);
			btnTimeOn2->tooltip(_("Press to update"));
			btnTimeOn2->box(FL_NO_BOX);
			btnTimeOn2->callback(cb_btnTimeOn);
			inpTimeOn2 = new Fl_Input2(
				pad + btnTimeOn2->x() + btnTimeOn2->w(), y,
				w_inpTime2, h, "");
			inpTimeOn2->tooltip(_("Time On"));
			inpTimeOn2->type(FL_INT_INPUT);

			const char *label3 = _("Off");
			Fl_Box *bx3 = new Fl_Box(pad + rightof(inpTimeOn2), y,
				static_cast<int>(fl_width(label3)), h, label3);
			inpTimeOff2 = new Fl_Input2(
				pad + bx3->x() + bx3->w(), y,
				w_inpTime2, h, "");
			inpTimeOff2->tooltip(_("Time Off"));
			inpTimeOff2->type(FL_NORMAL_OUTPUT);

			const char *label4 = _("Call");
			Fl_Box *bx4 = new Fl_Box(pad + rightof(inpTimeOff2), y,
				static_cast<int>(fl_width(label4)), h, label4);
			inpCall2 = new Fl_Input2(
				pad + bx4->x() + bx4->w(), y,
				w_inpCall2, h, "");
			inpCall2->tooltip(_("Other call"));

			const char *label6 = _("In");
			Fl_Box *bx6 = new Fl_Box(pad + rightof(inpCall2), y,
				static_cast<int>(fl_width(label6)), h, label6);
			inpRstIn2 = new Fl_Input2(
				pad + bx6->x() + bx6->w(), y,
				w_inpRstIn2, h, "");
			inpRstIn2->tooltip(_("Received RST"));

			const char *label7 = _("Out");
			Fl_Box *bx7 = new Fl_Box(pad + rightof(inpRstIn2), y,
				static_cast<int>(fl_width(label7)), h, label7);
			inpRstOut2 = new Fl_Input2(
				pad + bx7->x() + bx7->w(), y,
				w_inpRstOut2, h, "");
			inpRstOut2->tooltip(_("Sent RST"));

			const char *label5 = _("Nm");//_("Name");
			Fl_Box *bx5 = new Fl_Box(pad + rightof(inpRstOut2), y,
				static_cast<int>(fl_width(label5)), h, label5);
			int xn = pad + bx5->x() + bx5->w();
			inpName2 = new Fl_Input2(
				xn, y,
				progStatus.mainW - xn - pad, h, "");
			inpName2->tooltip(_("Other name"));

		}
		TopFrame2->resizable(inpName2);
		TopFrame2->end();
		TopFrame2->hide();

		TopFrame3 = new Fl_Group(0, Hmenu, progStatus.mainW, Hentry + 2 * pad);
		{
			int y = Hmenu + pad;
			int h = Hentry;
			qsoFreqDisp3 = new cFreqControl(
				pad, Hmenu,
				freqwidth, freqheight, "");
			qsoFreqDisp3->box(FL_DOWN_BOX);
			qsoFreqDisp3->color(FL_BACKGROUND_COLOR);
			qsoFreqDisp3->selection_color(FL_BACKGROUND_COLOR);
			qsoFreqDisp3->labeltype(FL_NORMAL_LABEL);
			qsoFreqDisp3->align(FL_ALIGN_CENTER);
			qsoFreqDisp3->when(FL_WHEN_RELEASE);
			qsoFreqDisp3->callback(qso_movFreq);
			qsoFreqDisp3->font(progdefaults.FreqControlFontnbr);
			qsoFreqDisp3->SetONOFFCOLOR(
				fl_rgb_color(	progdefaults.FDforeground.R,
								progdefaults.FDforeground.G,
								progdefaults.FDforeground.B),
				fl_rgb_color(	progdefaults.FDbackground.R,
								progdefaults.FDbackground.G,
								progdefaults.FDbackground.B));
			qsoFreqDisp3->value(0);

			qso_opPICK3 = new Fl_Button(
				rightof(qsoFreqDisp3), y,
				Wbtn, Hentry);
			qso_opPICK3->image(addrbookpixmap);
			qso_opPICK3->callback(showOpBrowserView2, 0);
			qso_opPICK3->tooltip(_("Open List"));

			qsoClear3 = new Fl_Button(
					pad + rightof(qso_opPICK3), y,
					Wbtn, Hentry);
			qsoClear3->image(new Fl_Pixmap(edit_clear_icon));
			qsoClear3->callback(qsoClear_cb, 0);
			qsoClear3->tooltip(_("Clear"));

			qsoSave3 = new Fl_Button(
					pad + rightof(qsoClear3), y,
					Wbtn, Hentry);
			qsoSave3->image(new Fl_Pixmap(save_icon));
			qsoSave3->callback(qsoSave_cb, 0);
			qsoSave3->tooltip(_("Save"));

			fl_font(FL_HELVETICA, FL_NORMAL_SIZE);
			const char *label2a = _("On");
			const char *label3a = _("Off");
			const char *label4a = _("Call");
			const char *label5a = _("# S");
			const char *label6a = _("# R");
			const char *label7a = _("Ex");
			const char *xData = "00000";
			const char *xCall = "WW8WWW/WWWW";
			int   wData = static_cast<int>(fl_width(xData));
			int   wCall = static_cast<int>(fl_width(xCall));

			Fl_Box *bx4a = new Fl_Box(
				pad + rightof(qsoSave3), y,
				static_cast<int>(fl_width(label4a)), h, label4a);
			inpCall3 = new Fl_Input2(
				pad + bx4a->x() + bx4a->w(), y,
				wCall, h, "");
			inpCall3->align(FL_ALIGN_INSIDE);
			inpCall3->tooltip(_("Other call"));

			Fl_Box *bx7a = new Fl_Box(
				rightof(inpCall3), y,
				static_cast<int>(fl_width(label7a)), h, label7a);
			bx7a->align(FL_ALIGN_INSIDE);
			inpXchgIn2 = new Fl_Input2(
				rightof(bx7a), y,
				static_cast<int>(progStatus.mainW 
				- rightof(bx7a) - pad
				- fl_width(label6a) - wData - pad
				- fl_width(label5a) - wData - pad
				- fl_width(label2a) - wData - pad
				- fl_width(label3a) - wData - pad), 
				h, "");
			inpXchgIn2->tooltip(_("Contest exchange in"));

			Fl_Box *bx6a = new Fl_Box(
				rightof(inpXchgIn2), y,
				static_cast<int>(fl_width(label6a)), h, label6a);
			bx6a->align(FL_ALIGN_INSIDE);
			inpSerNo2 = new Fl_Input2(
				rightof(bx6a) + pad, y,
				wData, h, "");
			inpSerNo2->tooltip(_("Received serial number"));

			Fl_Box *bx5a = new Fl_Box(
				rightof(inpSerNo2), y,
				static_cast<int>(fl_width(label5a)), h, label5a);
			bx5a->align(FL_ALIGN_INSIDE);
			outSerNo2 = new Fl_Input2(
				rightof(bx5a) + pad, y,
				wData, h, "");
			outSerNo2->tooltip(_("Sent serial number (read only)"));
			outSerNo2->type(FL_NORMAL_OUTPUT);

			btnTimeOn3 = new Fl_Button(
				rightof(outSerNo2), y,
				static_cast<int>(fl_width(label2a)), h, label2a);
			btnTimeOn3->tooltip(_("Press to update"));
			btnTimeOn3->box(FL_NO_BOX);
			btnTimeOn3->callback(cb_btnTimeOn);
			inpTimeOn3 = new Fl_Input2(
				btnTimeOn3->x() + btnTimeOn3->w() + pad, y,
				wData - 2, h, "");
			inpTimeOn3->tooltip(_("Time On"));
			inpTimeOn3->type(FL_INT_INPUT);

			Fl_Box *bx3a = new Fl_Box(pad + rightof(inpTimeOn3), y,
				static_cast<int>(fl_width(label3a)), h, label3a);
			inpTimeOff3 = new Fl_Input2(
				bx3a->x() + bx3a->w() + pad, y,
				wData, h, "");
			inpTimeOff3->tooltip(_("Time Off"));
			inpTimeOff3->type(FL_NORMAL_OUTPUT);

			TopFrame3->end();
		}
		TopFrame3->resizable(inpXchgIn2);
		TopFrame3->hide();

		inpFreq = inpFreq1;
		inpCall = inpCall1;
		inpTimeOn = inpTimeOn1;
		inpTimeOff = inpTimeOff1;
		inpName = inpName1;
		inpRstIn = inpRstIn1;
		inpRstOut = inpRstOut1;
		qsoFreqDisp = qsoFreqDisp1;
		inpSerNo = inpSerNo1;
		outSerNo = outSerNo1;
		inpXchgIn = inpXchgIn1;

		Y = Hmenu + Hqsoframe + pad;

		int Htext = progStatus.mainH - Hwfall - Hmenu - Hstatus - Hmacros - Hqsoframe - 4;
		int Hrcvtxt = (Htext) / 2;
		int Hxmttxt = (Htext - (Hrcvtxt));
		int sw = DEFAULT_SW;
		MixerFrame = new Fl_Group(0,Y,sw, Hrcvtxt + Hxmttxt);
			valRcvMixer = new Fl_Value_Slider2(0, Y, sw, (Htext)/2, "");
			valRcvMixer->type(FL_VERT_NICE_SLIDER);
			valRcvMixer->color(fl_rgb_color(0,110,30));
			valRcvMixer->labeltype(FL_ENGRAVED_LABEL);
			valRcvMixer->selection_color(fl_rgb_color(255,255,0));
			valRcvMixer->textcolor(FL_WHITE);
			valRcvMixer->range(100.0,0.0);
			valRcvMixer->value(100.0);
			valRcvMixer->step(1.0);
			valRcvMixer->callback( (Fl_Callback *)cb_RcvMixer);
			valXmtMixer = new Fl_Value_Slider2(0, Y + (Htext)/2, sw, (Htext)/2, "");
			valXmtMixer->type(FL_VERT_NICE_SLIDER);
			valXmtMixer->color(fl_rgb_color(110,0,30));
			valXmtMixer->labeltype(FL_ENGRAVED_LABEL);
			valXmtMixer->selection_color(fl_rgb_color(255,255,0));
			valXmtMixer->textcolor(FL_WHITE);
			valXmtMixer->range(100.0,0.0);
			valXmtMixer->value(100.0);
			valXmtMixer->step(1.0);
			valXmtMixer->callback( (Fl_Callback *)cb_XmtMixer);
		MixerFrame->end();

		TiledGroup = new Fl_Tile_Check(sw, Y, progStatus.mainW-sw, Htext);
			int minRxHeight = Hrcvtxt;
			int minTxHeight;
			if (minRxHeight < 66) minRxHeight = 66;
			minTxHeight = Htext - minRxHeight;

			ReceiveText = new FTextRX(sw, Y, progStatus.mainW-sw, minRxHeight, "");
			ReceiveText->color(
				fl_rgb_color(
					progdefaults.RxColor.R,
					progdefaults.RxColor.G,
					progdefaults.RxColor.B));
			ReceiveText->setFont(progdefaults.RxFontnbr);
			ReceiveText->setFontSize(progdefaults.RxFontsize);
			ReceiveText->setFontColor(progdefaults.RxFontcolor, FTextBase::RECV);
			ReceiveText->setFontColor(progdefaults.XMITcolor, FTextBase::XMIT);
			ReceiveText->setFontColor(progdefaults.CTRLcolor, FTextBase::CTRL);
			ReceiveText->setFontColor(progdefaults.SKIPcolor, FTextBase::SKIP);
			ReceiveText->setFontColor(progdefaults.ALTRcolor, FTextBase::ALTR);

			FHdisp = new Raster(sw, Y, progStatus.mainW-sw, minRxHeight);
			FHdisp->hide();

			TransmitText = new FTextTX(sw, Y + minRxHeight, progStatus.mainW-sw, minTxHeight);
			TransmitText->color(
				fl_rgb_color(
					progdefaults.TxColor.R,
					progdefaults.TxColor.G,
					progdefaults.TxColor.B));
			TransmitText->setFont(progdefaults.TxFontnbr);
			TransmitText->setFontSize(progdefaults.TxFontsize);
			TransmitText->setFontColor(progdefaults.TxFontcolor, FTextBase::RECV);
			TransmitText->setFontColor(progdefaults.XMITcolor, FTextBase::XMIT);
			TransmitText->setFontColor(progdefaults.CTRLcolor, FTextBase::CTRL);
			TransmitText->setFontColor(progdefaults.SKIPcolor, FTextBase::SKIP);
			TransmitText->setFontColor(progdefaults.ALTRcolor, FTextBase::ALTR);

			Fl_Box *minbox = new Fl_Box(sw,Y + 66, progStatus.mainW-sw, Htext - 66 - 66);
			minbox->hide();

			TiledGroup->resizable(minbox);

			Y += Htext;

		TiledGroup->end();
		Fl_Group::current()->resizable(TiledGroup);

		Fl::add_handler(default_handler);

		Fl_Box *bx;
		macroFrame = new Fl_Box(0, Y, progStatus.mainW, Hmacros);
			macroFrame->box(FL_ENGRAVED_FRAME);
			int Wbtn = (progStatus.mainW - 30 - 8 - 4)/NUMMACKEYS;
			int xpos = 2;
			for (int i = 0; i < NUMMACKEYS; i++) {
				if (i == 4 || i == 8) {
					bx = new Fl_Box(xpos, Y+2, 5, Hmacros - 4);
					bx->box(FL_FLAT_BOX);
					bx->color(FL_BLACK);
					xpos += 4;
				}
				btnMacro[i] = new Fl_Button(xpos, Y+2, Wbtn, Hmacros - 4, macros.name[i].c_str());
				btnMacro[i]->callback(macro_cb, (void *)i);
				btnMacro[i]->tooltip(_("Left Click - execute\nRight Click - edit"));
				colorize_macro(i);
				xpos += Wbtn;
			}
			bx = new Fl_Box(xpos, Y+2, progStatus.mainW - 32 - xpos, Hmacros - 4);
			bx->box(FL_FLAT_BOX);
			bx->color(FL_BLACK);
			btnAltMacros = new Fl_Button(progStatus.mainW-32, Y+2, 30, Hmacros - 4, "1");
			btnAltMacros->callback(altmacro_cb, 0);
			btnAltMacros->tooltip(_("Change macro set"));

		Y += Hmacros;
		Fl_Pack *wfpack = new Fl_Pack(0, Y, progStatus.mainW, Hwfall);
			wfpack->type(1);

			wf = new waterfall(0, Y, Wwfall, Hwfall);
			wf->end();

			pgrsSquelch = new Progress(
				rightof(wf), Y + 4,
				DEFAULT_SW, Hwfall - 8,
				"");
			pgrsSquelch->color(FL_BACKGROUND2_COLOR, FL_DARK_GREEN);
			pgrsSquelch->type(Progress::VERTICAL);
			pgrsSquelch->tooltip(_("Detected signal level"));
				sldrSquelch = new Fl_Slider2(
				rightof(pgrsSquelch), Y + 4,
				DEFAULT_SW, Hwfall - 8,
				"");
			sldrSquelch->minimum(100);
			sldrSquelch->maximum(0);
			sldrSquelch->step(1);
			sldrSquelch->value(progStatus.sldrSquelchValue);
			sldrSquelch->callback((Fl_Callback*)cb_sldrSquelch);
			sldrSquelch->color(FL_INACTIVE_COLOR);
			sldrSquelch->tooltip(_("Squelch level"));
				Fl_Group::current()->resizable(wf);
		wfpack->end();

		Y += (Hwfall + 2);

		Fl_Pack *hpack = new Fl_Pack(0, Y, progStatus.mainW, Hstatus);
			hpack->type(1);
			MODEstatus = new Fl_Button(0,Hmenu+Hrcvtxt+Hxmttxt+Hwfall, Wmode+30, Hstatus, "");
			MODEstatus->box(FL_DOWN_BOX);
			MODEstatus->color(FL_BACKGROUND2_COLOR);
			MODEstatus->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
			MODEstatus->callback(status_cb, (void *)0);
			MODEstatus->when(FL_WHEN_CHANGED);
			MODEstatus->tooltip(_("Left click: change mode\nRight click: configure"));

			cntCW_WPM = new Fl_Counter2(rightof(MODEstatus), Hmenu+Hrcvtxt+Hxmttxt+Hwfall, 
				Ws2n - Hstatus, Hstatus, "");
			cntCW_WPM->callback(cb_cntCW_WPM);
			cntCW_WPM->minimum(progdefaults.CWlowerlimit);
			cntCW_WPM->maximum(progdefaults.CWupperlimit);
			cntCW_WPM->value(progdefaults.CWspeed);
			cntCW_WPM->type(1);
			cntCW_WPM->step(1);
			cntCW_WPM->tooltip(_("CW transmit WPM"));
			cntCW_WPM->hide();

			btnCW_Default = new Fl_Button(rightof(cntCW_WPM), Hmenu+Hrcvtxt+Hxmttxt+Hwfall,
				Hstatus, Hstatus, "*");
			btnCW_Default->callback(cb_btnCW_Default);
			btnCW_Default->tooltip(_("Default WPM"));
			btnCW_Default->hide();

			Status1 = new Fl_Box(rightof(MODEstatus), Hmenu+Hrcvtxt+Hxmttxt+Hwfall, Ws2n, Hstatus, "");
			Status1->box(FL_DOWN_BOX);
			Status1->color(FL_BACKGROUND2_COLOR);
			Status1->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);

			Status2 = new Fl_Box(rightof(Status1), Hmenu+Hrcvtxt+Hxmttxt+Hwfall, Wimd, Hstatus, "");
			Status2->box(FL_DOWN_BOX);
			Status2->color(FL_BACKGROUND2_COLOR);
			Status2->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);

			inpCall4 = new Fl_Input2(
				rightof(Status1), Hmenu+Hrcvtxt+Hxmttxt+Hwfall,
				Wimd, Hstatus, "");//"Callsign:");
			inpCall4->align(FL_ALIGN_LEFT);
			inpCall4->tooltip(_("Other call"));
			inpCall4->hide();

			StatusBar = new Fl_Box(
                rightof(Status2), Hmenu+Hrcvtxt+Hxmttxt+Hwfall,
                progStatus.mainW - bwSqlOnOff - bwAfcOnOff - Wwarn - rightof(Status2) - 2 * pad,// - 60,
                Hstatus, "");
			StatusBar->box(FL_DOWN_BOX);
			StatusBar->color(FL_BACKGROUND2_COLOR);
			StatusBar->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);

			WARNstatus = new Fl_Box(
				rightof(StatusBar) + pad, Hmenu+Hrcvtxt+Hxmttxt+Hwfall,
                Wwarn, Hstatus, "");
			WARNstatus->box(FL_DIAMOND_DOWN_BOX);
			WARNstatus->color(FL_BACKGROUND_COLOR);
			WARNstatus->labelcolor(FL_RED);
			WARNstatus->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);

			int sql_width = bwSqlOnOff;
#ifdef __APPLE__
			sql_width -= 15; // leave room for resize handle
#endif
			btnAFC = new Fl_Light_Button(
							progStatus.mainW - bwSqlOnOff - bwAfcOnOff,
							Hmenu+Hrcvtxt+Hxmttxt+Hwfall,
							bwAfcOnOff, Hstatus, "AFC");
			btnAFC->selection_color(progdefaults.AfcColor);
			btnSQL = new Fl_Light_Button(
							progStatus.mainW - bwSqlOnOff,
							Hmenu+Hrcvtxt+Hxmttxt+Hwfall,
							sql_width, Hstatus, "SQL");
			btnSQL->selection_color(progdefaults.Sql1Color);

			btnAFC->callback(cbAFC, 0);
			btnAFC->value(1);
			btnAFC->tooltip(_("Automatic Frequency Control"));
			btnSQL->callback(cbSQL, 0);
			btnSQL->value(1);
			btnSQL->tooltip(_("Squelch"));


			Fl_Group::current()->resizable(StatusBar);
		hpack->end();

		showMacroSet();

#define CB_WHEN FL_WHEN_CHANGED | FL_WHEN_NOT_CHANGED | FL_WHEN_ENTER_KEY | FL_WHEN_RELEASE
		Fl_Widget* logfields[] = {
			inpName1, inpName1,
			inpTimeOn1, inpTimeOn2, inpTimeOn3,
			inpTimeOff1, inpTimeOff2, inpTimeOff3,
			inpRstIn1, inpRstIn2,
			inpRstOut1, inpRstOut2,
			inpQth, inpState, inpVEprov, inpCountry, inpAZ, inpNotes,
			inpSerNo1, inpSerNo2,
			outSerNo1, outSerNo2,
			inpXchgIn1, inpXchgIn2 };
		for (size_t i = 0; i < sizeof(logfields)/sizeof(*logfields); i++) {
			logfields[i]->callback(cb_log);
			logfields[i]->when(CB_WHEN);
		}
		// exceptions
		inpCall1->callback(cb_call);
		inpCall1->when(CB_WHEN);
		inpCall2->callback(cb_call);
		inpCall2->when(CB_WHEN);
		inpCall3->callback(cb_call);
		inpCall3->when(CB_WHEN);
		inpCall4->callback(cb_call);
		inpCall4->when(CB_WHEN);

		inpLoc->callback(cb_loc);
		inpLoc->when(CB_WHEN);

		inpNotes->when(FL_WHEN_RELEASE);

	fl_digi_main->end();
	fl_digi_main->callback(cb_wMain);

	scopeview = new Fl_Double_Window(0,0,140,140, _("Scope"));
	scopeview->xclass(PACKAGE_NAME);
	digiscope = new Digiscope (0, 0, 140, 140);
	scopeview->resizable(digiscope);
	scopeview->size_range(SCOPEWIN_MIN_WIDTH, SCOPEWIN_MIN_HEIGHT);
	scopeview->end();
	scopeview->hide();

	ssdv = new ssdv_rx(320, 240 + 60, _("SSDV RX"));
	ssdv->xclass(PACKAGE_NAME);
	ssdv->hide();

	if (!progdefaults.menuicons)
		toggle_icon_labels();

	// ztimer must be run by FLTK's timeout handler
	Fl::add_timeout(0.0, ztimer, (void*)true);

	// Set the state of checked toggle menu items

	struct {
		bool var; const char* label;
	} toggles[] = {
		{ progStatus.LOGenabled, LOG_TO_FILE_MLABEL },
		{ progStatus.contest, CONTEST_FIELDS_MLABEL },
		{ progStatus.WF_UI, WF_MLABEL },
		{ progStatus.Rig_Log_UI, RIGLOG_MLABEL },
		{ progStatus.Rig_Contest_UI, RIGCONTEST_MLABEL },
		{ progStatus.NO_RIGLOG, RIGLOG_NONE_MLABEL },
		{ progStatus.DOCKEDSCOPE, DOCKEDSCOPE_MLABEL }
	};
	Fl_Menu_Item* toggle;
	for (size_t i = 0; i < sizeof(toggles)/sizeof(*toggles); i++) {
		if (toggles[i].var && (toggle = getMenuItem(toggles[i].label))) {
			toggle->set();
			if (toggle->callback()) {
				mnu->value(toggle);
				toggle->do_callback(reinterpret_cast<Fl_Widget*>(mnu));
			}
		}
	}
	if (!dxcc_is_open())
		getMenuItem(COUNTRIES_MLABEL)->hide();

        /* TODO: REMOVE ME: SEE LINE 2471 */ 
	  set_menu_dl_online();
	/* TODO */

	UI_select();
	wf->UI_select(progStatus.WF_UI);

	createConfig();
	if (withnoise)
		grpNoise->show();
}

void cb_mnuAltDockedscope(Fl_Menu_ *w, void *d);
void cb_mnuCaptureHAB(Fl_Widget *w, void *d);
void cb_mnuGenerateHAB(Fl_Widget *w, void *d);
void cb_mnuPlaybackHAB(Fl_Widget *w, void *d);

Fl_Menu_Item alt_menu_[] = {
{_("&File"), 0,  0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},

#if USE_SNDFILE
{ make_icon_label(_("Audio")), 0, 0, 0, FL_SUBMENU, _FL_MULTI_LABEL, 0, 14, 0},
{_("RX capture"),  0, (Fl_Callback*)cb_mnuCaptureHAB,  0, FL_MENU_TOGGLE, FL_NORMAL_LABEL, 0, 14, 0},
{_("TX generate"), 0, (Fl_Callback*)cb_mnuGenerateHAB, 0, FL_MENU_TOGGLE, FL_NORMAL_LABEL, 0, 14, 0},
{_("Playback"),    0, (Fl_Callback*)cb_mnuPlaybackHAB, 0, FL_MENU_TOGGLE, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},
#endif

{ make_icon_label(_("Exit"), log_out_icon), 'x',  (Fl_Callback*)cb_E, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{_("Op &Mode"), 0,  0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},

{ mode_info[MODE_CW].name, 0, cb_init_mode, (void *)MODE_CW, 0, FL_NORMAL_LABEL, 0, 14, 0},

{"Contestia", 0, 0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},
{ "4/250", 0, cb_contestiaA, (void *)MODE_CONTESTIA, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ "8/250", 0, cb_contestiaB, (void *)MODE_CONTESTIA, FL_MENU_DIVIDER, FL_NORMAL_LABEL, 0, 14, 0},
{ "4/500", 0, cb_contestiaC, (void *)MODE_CONTESTIA, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ "8/500", 0, cb_contestiaD, (void *)MODE_CONTESTIA, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ "16/500", 0, cb_contestiaE, (void *)MODE_CONTESTIA, FL_MENU_DIVIDER, FL_NORMAL_LABEL, 0, 14, 0},
{ "8/1000", 0, cb_contestiaF, (void *)MODE_CONTESTIA, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ "16/1000", 0, cb_contestiaG, (void *)MODE_CONTESTIA, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ "32/1000", 0, cb_contestiaH, (void *)MODE_CONTESTIA, FL_MENU_DIVIDER, FL_NORMAL_LABEL, 0, 14, 0},
{ _("Custom..."), 0, cb_contestiaCustom, (void *)MODE_CONTESTIA, 0, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{"DominoEX", 0, 0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_DOMINOEX4].name, 0, cb_init_mode, (void *)MODE_DOMINOEX4, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_DOMINOEX5].name, 0, cb_init_mode, (void *)MODE_DOMINOEX5, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_DOMINOEX8].name, 0, cb_init_mode, (void *)MODE_DOMINOEX8, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_DOMINOEX11].name, 0, cb_init_mode, (void *)MODE_DOMINOEX11, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_DOMINOEX16].name, 0, cb_init_mode, (void *)MODE_DOMINOEX16, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_DOMINOEX22].name, 0, cb_init_mode, (void *)MODE_DOMINOEX22, 0, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{"MFSK", 0, 0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_MFSK4].name, 0,  cb_init_mode, (void *)MODE_MFSK4, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_MFSK8].name, 0,  cb_init_mode, (void *)MODE_MFSK8, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_MFSK11].name, 0,  cb_init_mode, (void *)MODE_MFSK11, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_MFSK16].name, 0,  cb_init_mode, (void *)MODE_MFSK16, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_MFSK22].name, 0,  cb_init_mode, (void *)MODE_MFSK22, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_MFSK31].name, 0,  cb_init_mode, (void *)MODE_MFSK31, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_MFSK32].name, 0,  cb_init_mode, (void *)MODE_MFSK32, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_MFSK64].name, 0,  cb_init_mode, (void *)MODE_MFSK64, 0, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{"MT63", 0, 0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_MT63_500].name, 0,  cb_init_mode, (void *)MODE_MT63_500, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_MT63_1000].name, 0,  cb_init_mode, (void *)MODE_MT63_1000, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_MT63_2000].name, 0,  cb_init_mode, (void *)MODE_MT63_2000, 0, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{"Olivia", 0, 0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},
{ "8/250", 0, cb_oliviaA, (void *)MODE_OLIVIA, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ "8/500", 0, cb_oliviaB, (void *)MODE_OLIVIA, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ "16/500", 0, cb_oliviaC, (void *)MODE_OLIVIA, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ "8/1000", 0, cb_oliviaD, (void *)MODE_OLIVIA, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ "32/1000", 0, cb_oliviaE, (void *)MODE_OLIVIA, FL_MENU_DIVIDER, FL_NORMAL_LABEL, 0, 14, 0},
{ _("Custom..."), 0, cb_oliviaCustom, (void *)MODE_OLIVIA, 0, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{"PSK", 0, 0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_PSK31].name, 0, cb_init_mode, (void *)MODE_PSK31, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_PSK63].name, 0, cb_init_mode, (void *)MODE_PSK63, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_PSK63F].name, 0, cb_init_mode, (void *)MODE_PSK63F, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_PSK125].name, 0, cb_init_mode, (void *)MODE_PSK125, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_PSK250].name, 0, cb_init_mode, (void *)MODE_PSK250, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_PSK500].name, 0, cb_init_mode, (void *)MODE_PSK500, 0, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{"QPSK", 0, 0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_QPSK31].name, 0, cb_init_mode, (void *)MODE_QPSK31, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_QPSK63].name, 0, cb_init_mode, (void *)MODE_QPSK63, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_QPSK125].name, 0, cb_init_mode, (void *)MODE_QPSK125, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_QPSK250].name, 0, cb_init_mode, (void *)MODE_QPSK250, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_QPSK500].name, 0, cb_init_mode, (void *)MODE_QPSK500, 0, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{"PSKR", 0, 0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_PSK125R].name, 0, cb_init_mode, (void *)MODE_PSK125R, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_PSK250R].name, 0, cb_init_mode, (void *)MODE_PSK250R, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_PSK500R].name, 0, cb_init_mode, (void *)MODE_PSK500R, 0, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{"RTTY", 0, 0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},
{ "RTTY-45", 0, cb_rtty45, (void *)MODE_RTTY, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ "RTTY-50", 0, cb_rtty50, (void *)MODE_RTTY, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ "RTTY-HAB-50", 0, cb_rttyHAB50, (void *)MODE_RTTY, 0, FL_NORMAL_LABEL, 0, 14, 0}, 
{ "RTTY-75", 0, cb_rtty75, (void *)MODE_RTTY, FL_MENU_DIVIDER, FL_NORMAL_LABEL, 0, 14, 0},
{ _("Custom..."), 0, cb_rttyCustom, (void *)MODE_RTTY, 0, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{"THOR", 0, 0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_THOR4].name, 0, cb_init_mode, (void *)MODE_THOR4, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_THOR5].name, 0, cb_init_mode, (void *)MODE_THOR5, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_THOR8].name, 0, cb_init_mode, (void *)MODE_THOR8, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_THOR11].name, 0, cb_init_mode, (void *)MODE_THOR11, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_THOR16].name, 0, cb_init_mode, (void *)MODE_THOR16, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_THOR22].name, 0, cb_init_mode, (void *)MODE_THOR22, 0, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{"Throb", 0, 0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_THROB1].name, 0, cb_init_mode, (void *)MODE_THROB1, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_THROB2].name, 0, cb_init_mode, (void *)MODE_THROB2, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_THROB4].name, 0, cb_init_mode, (void *)MODE_THROB4, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_THROBX1].name, 0, cb_init_mode, (void *)MODE_THROBX1, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_THROBX2].name, 0, cb_init_mode, (void *)MODE_THROBX2, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_THROBX4].name, 0, cb_init_mode, (void *)MODE_THROBX4, 0, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{"NBEMS modes", 0, 0, 0, FL_SUBMENU | FL_MENU_DIVIDER, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_DOMINOEX11].name, 0, cb_init_mode, (void *)MODE_DOMINOEX11, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_DOMINOEX22].name, 0, cb_init_mode, (void *)MODE_DOMINOEX22, FL_MENU_DIVIDER, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_MFSK16].name, 0,  cb_init_mode, (void *)MODE_MFSK16, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_MFSK32].name, 0,  cb_init_mode, (void *)MODE_MFSK32, FL_MENU_DIVIDER, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_PSK125].name, 0, cb_init_mode, (void *)MODE_PSK125, 0, FL_NORMAL_LABEL, 0, 14, 0},
{ mode_info[MODE_PSK250].name, 0, cb_init_mode, (void *)MODE_PSK250, 0, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{ mode_info[MODE_SSB].name, 0, cb_init_mode, (void *)MODE_SSB, 0, FL_NORMAL_LABEL, 0, 14, 0},

{ mode_info[MODE_WWV].name, 0, cb_init_mode, (void *)MODE_WWV, 0, FL_NORMAL_LABEL, 0, 14, 0},

{ mode_info[MODE_ANALYSIS].name, 0, cb_init_mode, (void *)MODE_ANALYSIS, 0, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{_("&Configure"), 0, 0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},
{ make_icon_label(_("Waterfall"), waterfall_icon), 0,  (Fl_Callback*)cb_mnuConfigWaterfall, 0, FL_MENU_DIVIDER, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(RIGCONTROL_MLABEL, multimedia_player_icon), 0, (Fl_Callback*)cb_mnuConfigRigCtrl, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("Sound Card"), audio_card_icon), 0, (Fl_Callback*)cb_mnuConfigSoundCard, 0, FL_MENU_DIVIDER, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("Modems"), emblems_system_icon), 0, (Fl_Callback*)cb_mnuConfigModems, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("IDs")), 0,  (Fl_Callback*)cb_mnuConfigID, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("Notifications")), 0,  (Fl_Callback*)cb_mnuConfigNotify, 0, FL_MENU_DIVIDER, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("Save Config"), save_icon), 0, (Fl_Callback*)cb_mnuSaveConfig, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{ VIEW_MLABEL, 0, 0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},
//{ make_icon_label(_("Extern Scope"), utilities_system_monitor_icon), 'd', (Fl_Callback*)cb_mnuDigiscope, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(MFSK_IMAGE_MLABEL, image_icon), 'm', (Fl_Callback*)cb_mnuPicViewer, 0, FL_MENU_INACTIVE, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("PSK Browser")), 'p', (Fl_Callback*)cb_mnuViewer, 0, FL_MENU_DIVIDER, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("SSDV RX")), 's', (Fl_Callback*)cb_mnuShowSSDVRX, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{ DOCKEDSCOPE_MLABEL, 0, (Fl_Callback*)cb_mnuAltDockedscope, 0, FL_MENU_TOGGLE, FL_NORMAL_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{_("DL Client"), 0, 0, 0, FL_SUBMENU, FL_NORMAL_LABEL, 0, 14, 0},
{ DLFLDIGI_ONLINE_LABEL, 0, cb_toggle_dl_online, 0, FL_MENU_TOGGLE, FL_NORMAL_LABEL, 0, 14, 0},
{ make_icon_label(_("Refresh Payload Data"), pskr_icon), 0, cb_dl_fldigi_refresh, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("Configure"), help_about_icon), 0, (Fl_Callback*)cb_mnuConfigDLClient, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("Tracker"), pskr_icon), 0, cb_mnuVisitTracker, 0, FL_MENU_DIVIDER, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("Raw Data"), pskr_icon), 0, cb_mnuVisitView, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{ make_icon_label(_("Help"), pskr_icon), 0, cb_mnuVisitDLClient, 0, 0, _FL_MULTI_LABEL, 0, 14, 0},
{0,0,0,0,0,0,0,0,0},

{0,0,0,0,0,0,0,0,0},
};

void cb_mnuAltDockedscope(Fl_Menu_ *w, void *d) {
	Fl_Menu_Item *m = getMenuItem(((Fl_Menu_*)w)->mvalue()->label(), alt_menu_);
	progStatus.DOCKEDSCOPE = m->value();
	wf->show_scope(progStatus.DOCKEDSCOPE);
}
#if USE_SNDFILE
void cb_mnuCaptureHAB(Fl_Widget *w, void *d)
{
	if (!scard) return;
	Fl_Menu_Item *m = getMenuItem(((Fl_Menu_*)w)->mvalue()->label(), alt_menu_); //eek
	if (playval || genval) {
		m->clear();
		return;
	}
	capval = m->value();
	if(!scard->Capture(capval)) {
		m->clear();
		capval = false;
	}
}

void cb_mnuGenerateHAB(Fl_Widget *w, void *d)
{
	if (!scard) return;
	Fl_Menu_Item *m = getMenuItem(((Fl_Menu_*)w)->mvalue()->label(), alt_menu_);
	if (capval || playval) {
		m->clear();
		return;
	}
	genval = m->value();
	if (!scard->Generate(genval)) {
		m->clear();
		genval = false;
	}
}
void cb_mnuPlaybackHAB(Fl_Widget *w, void *d)
{
	if (!scard) {
	 return;
	 }
	Fl_Menu_Item *m = getMenuItem(((Fl_Menu_*)w)->mvalue()->label(), alt_menu_);
	if (capval || genval) {
		m->clear();
		return;
	}
	playval = m->value();
	cout << playval << endl;
	if(!scard->Playback(playval)) {
		m->clear();
		playval = false;
	}
	else if (btnAutoSpot->value()) {
		put_status(_("Spotting disabled"), 3.0);
		btnAutoSpot->value(0);
		btnAutoSpot->do_callback();
	}
}
#endif // USE_SNDFILE


#define defwidget 0, 0, 10, 10, ""

void noop_controls() // create and then hide all controls not being used
{
	Fl_Double_Window *dummywindow = new Fl_Double_Window(0,0,100,100,"");

	btnMacroTimer = new Fl_Button(defwidget); btnMacroTimer->hide();

	if(bWF_only) {
		ReceiveText = new FTextRX(0,0,100,100); ReceiveText->hide();
	}
	TransmitText = new FTextTX(0,0,100,100); TransmitText->hide();
	FHdisp = new Raster(0,0,10,100); FHdisp->hide();

	for (int i = 0; i < NUMMACKEYS; i++) {
		btnMacro[i] = new Fl_Button(defwidget); btnMacro[i]->hide();
	}
	btnAltMacros = new Fl_Button(defwidget); btnAltMacros->hide();

	inpQth = new Fl_Input2(defwidget); inpQth->hide();
	inpLoc = new Fl_Input2(defwidget); inpLoc->hide();
	inpState = new Fl_Input2(defwidget); inpState->hide();
	inpCountry = new Fl_Input2(defwidget); inpCountry->hide();
	inpSerNo = new Fl_Input2(defwidget); inpSerNo->hide();
	outSerNo = new Fl_Input2(defwidget); outSerNo->hide();
	inpXchgIn = new Fl_Input2(defwidget); inpXchgIn->hide();
	inpVEprov = new Fl_Input2(defwidget); inpVEprov->hide();
	inpNotes = new Fl_Input2(defwidget); inpNotes->hide();
	inpAZ = new Fl_Input2(defwidget); inpAZ->hide();

	qsoTime = new Fl_Button(defwidget); qsoTime->hide();
	btnQRZ = new Fl_Button(defwidget); btnQRZ->hide();
	qsoClear = new Fl_Button(defwidget); qsoClear->hide();
	qsoSave = new Fl_Button(defwidget); qsoSave->hide();

	txtRigName = new Fl_Box(defwidget); txtRigName->hide();
	qsoFreqDisp = new cFreqControl(0,0,100,10,""); qsoFreqDisp->hide();
	qso_opMODE = new Fl_ComboBox(defwidget); qso_opMODE->hide();
	qso_opBW = new Fl_ComboBox(defwidget); qso_opBW->hide();
	qso_opPICK = new Fl_Button(defwidget); qso_opPICK->hide();

	inpFreq = new Fl_Input2(defwidget); inpFreq->hide();
	inpTimeOff = new Fl_Input2(defwidget); inpTimeOff->hide();
	inpTimeOn = new Fl_Input2(defwidget); inpTimeOn->hide();
	btnTimeOn = new Fl_Button(defwidget); btnTimeOn->hide();
	inpCall = new Fl_Input2(defwidget); inpCall->hide();
	inpName = new Fl_Input2(defwidget); inpName->hide();
	inpRstIn = new Fl_Input2(defwidget); inpRstIn->hide();
	inpRstOut = new Fl_Input2(defwidget); inpRstOut->hide();

	inpFreq1 = new Fl_Input2(defwidget); inpFreq1->hide();
	inpTimeOff1 = new Fl_Input2(defwidget); inpTimeOff1->hide();
	inpTimeOn1 = new Fl_Input2(defwidget); inpTimeOn1->hide();
	btnTimeOn1 = new Fl_Button(defwidget); btnTimeOn1->hide();
	inpCall1 = new Fl_Input2(defwidget); inpCall1->hide();
	inpName1 = new Fl_Input2(defwidget); inpName1->hide();
	inpRstIn1 = new Fl_Input2(defwidget); inpRstIn1->hide();
	inpRstOut1 = new Fl_Input2(defwidget); inpRstOut1->hide();
	inpXchgIn1 = new Fl_Input2(defwidget); inpXchgIn1->hide();
	outSerNo1 = new Fl_Input2(defwidget); outSerNo1->hide();
	inpSerNo1 = new Fl_Input2(defwidget); inpSerNo1->hide();
	qsoFreqDisp1 = new cFreqControl(defwidget); qsoFreqDisp1->hide();

	inpFreq2 = new Fl_Input2(defwidget); inpFreq2->hide();
	inpTimeOff2 = new Fl_Input2(defwidget); inpTimeOff2->hide();
	inpTimeOn2 = new Fl_Input2(defwidget); inpTimeOn2->hide();
	btnTimeOn2 = new Fl_Button(defwidget); btnTimeOn2->hide();
	inpCall2 = new Fl_Input2(defwidget); inpCall2->hide();
	inpName2 = new Fl_Input2(defwidget); inpName2->hide();
	inpRstIn2 = new Fl_Input2(defwidget); inpRstIn2->hide();
	inpRstOut2 = new Fl_Input2(defwidget); inpRstOut2->hide();
	qsoFreqDisp2 = new cFreqControl(defwidget); qsoFreqDisp2->hide();

	qso_opPICK2 = new Fl_Button(defwidget); qso_opPICK2->hide();
	qsoClear2 = new Fl_Button(defwidget); qsoClear2->hide();
	qsoSave2 = new Fl_Button(defwidget); qsoSave2->hide();
	btnQRZ2 = new Fl_Button(defwidget); btnQRZ2->hide();

	inpTimeOff3 = new Fl_Input2(defwidget); inpTimeOff3->hide();
	inpTimeOn3 = new Fl_Input2(defwidget); inpTimeOn3->hide();
	btnTimeOn3 = new Fl_Button(defwidget); btnTimeOn3->hide();
	inpCall3 = new Fl_Input2(defwidget); inpCall3->hide();
	outSerNo2 = new Fl_Input2(defwidget); outSerNo2->hide();
	inpSerNo2 = new Fl_Input2(defwidget); inpSerNo2->hide();
	inpXchgIn2 = new Fl_Input2(defwidget); inpXchgIn2->hide();
	qsoFreqDisp3 = new cFreqControl(defwidget); qsoFreqDisp3->hide();

	qso_opPICK3 = new Fl_Button(defwidget); qso_opPICK3->hide();
	qsoClear3 = new Fl_Button(defwidget); qsoClear3->hide();
	qsoSave3 = new Fl_Button(defwidget); qsoSave3->hide();

	inpCall4 = new Fl_Input2(defwidget); inpCall4->hide();

	qso_opBrowser = new Fl_Browser(defwidget); qso_opBrowser->hide();
	qso_btnAddFreq = new Fl_Button(defwidget); qso_btnAddFreq->hide();
	qso_btnSelFreq = new Fl_Button(defwidget); qso_btnSelFreq->hide();
	qso_btnDelFreq = new Fl_Button(defwidget); qso_btnDelFreq->hide();
	qso_btnClearList = new Fl_Button(defwidget); qso_btnClearList->hide();
	qso_btnAct = new Fl_Button(defwidget); qso_btnAct->hide();
	qso_inpAct = new Fl_Input2(defwidget); qso_inpAct->hide();

	valRcvMixer = new Fl_Value_Slider2(defwidget); valRcvMixer->hide();
	valXmtMixer = new Fl_Value_Slider2(defwidget); valXmtMixer->hide();

	dummywindow->end();
	dummywindow->hide();

}

void make_scopeviewer()
{
	scopeview = new Fl_Double_Window(0,0,140,140, _("Scope"));
	scopeview->xclass(PACKAGE_NAME);
	digiscope = new Digiscope (0, 0, 140, 140);
	scopeview->resizable(digiscope);
	scopeview->size_range(SCOPEWIN_MIN_WIDTH, SCOPEWIN_MIN_HEIGHT);
	scopeview->end();
	scopeview->hide();
}

void altTabs()
{
	tabsConfigure->remove(tabUI);
	tabsConfigure->remove(tabFeld);
	tabsConfigure->remove(tabMisc);
	tabsConfigure->remove(tabQRZ);
}

int WF_only_height = 0;

void create_fl_digi_main_WF_only() {

	int fnt = fl_font();
	int fsize = fl_size();
	int freqheight = Hentry + 2 * pad;
	int Y = 0;

	fl_font(fnt, freqheight);
	fl_font(fnt, fsize);


	IMAGE_WIDTH = 4000;//progdefaults.HighFreqCutoff;
	Hwfall = progdefaults.wfheight;
	Wwfall = progStatus.mainW - 2 * DEFAULT_SW - 2 * pad;
	WF_only_height = Hmenu + Hwfall + Hstatus + 4 * pad;

	fl_digi_main = new Fl_Double_Window(progStatus.mainW, WF_only_height);

		mnuFrame = new Fl_Group(0,0,progStatus.mainW, Hmenu);

			mnu = new Fl_Menu_Bar(0, 0, progStatus.mainW - 200 - pad, Hmenu);
// do some more work on the menu
			for (size_t i = 0; i < sizeof(alt_menu_)/sizeof(alt_menu_[0]); i++) {
// FL_NORMAL_SIZE may have changed; update the menu items
				if (alt_menu_[i].text) {
					alt_menu_[i].labelsize_ = FL_NORMAL_SIZE;
				}
// set the icon label for items with the multi label type
				if (alt_menu_[i].labeltype() == _FL_MULTI_LABEL)
					set_icon_label(&alt_menu_[i]);
			}
			mnu->menu(alt_menu_);

			btnAutoSpot = new Fl_Light_Button(progStatus.mainW - 200 - pad, 0, 50, Hmenu, "Spot");
			btnAutoSpot->selection_color(progdefaults.SpotColor);
			btnAutoSpot->callback(cbAutoSpot, 0);
			btnAutoSpot->deactivate();

			btnRSID = new Fl_Light_Button(progStatus.mainW - 150 - pad, 0, 50, Hmenu, "RxID");
			btnRSID->selection_color(progdefaults.RxIDColor);
			btnRSID->tooltip("Receive RSID");
			btnRSID->callback(cbRSID, 0);

			btnTxRSID = new Fl_Light_Button(progStatus.mainW - 100 - pad, 0, 50, Hmenu, "TxID");
			btnTxRSID->selection_color(progdefaults.TxIDColor);
			btnTxRSID->tooltip("Transmit RSID");
			btnTxRSID->callback(cbTxRSID, 0);

			btnTune = new Fl_Light_Button(progStatus.mainW - 50 - pad, 0, 50, Hmenu, "TUNE");
			btnTune->selection_color(progdefaults.TuneColor);
			btnTune->callback(cbTune, 0);

		mnuFrame->resizable(mnu);
		mnuFrame->end();

		Y = Hmenu + pad;

		Fl_Pack *wfpack = new Fl_Pack(0, Y, progStatus.mainW, Hwfall);
			wfpack->type(1);
			wf = new waterfall(0, Y, Wwfall, Hwfall);
			wf->end();

			pgrsSquelch = new Progress(
				rightof(wf), Y + pad,
				DEFAULT_SW, Hwfall - 2 * pad,
				"");
			pgrsSquelch->color(FL_BACKGROUND2_COLOR, FL_DARK_GREEN);
			pgrsSquelch->type(Progress::VERTICAL);
			pgrsSquelch->tooltip(_("Detected signal level"));

			sldrSquelch = new Fl_Slider2(
				rightof(pgrsSquelch), Y + pad,
				DEFAULT_SW, Hwfall - 2 * pad,
				"");
			sldrSquelch->minimum(100);
			sldrSquelch->maximum(0);
			sldrSquelch->step(1);
			sldrSquelch->value(progStatus.sldrSquelchValue);
			sldrSquelch->callback((Fl_Callback*)cb_sldrSquelch);
			sldrSquelch->color(FL_INACTIVE_COLOR);
			sldrSquelch->tooltip(_("Squelch level"));
			Fl_Group::current()->resizable(wf);
		wfpack->end();

		Y += (Hwfall + pad);

		Fl_Pack *hpack = new Fl_Pack(0, Y, progStatus.mainW, Hstatus);
			hpack->type(1);
			MODEstatus = new Fl_Button(0, Y, Wmode+30, Hstatus, "");
			MODEstatus->box(FL_DOWN_BOX);
			MODEstatus->color(FL_BACKGROUND2_COLOR);
			MODEstatus->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
			MODEstatus->callback(status_cb, (void *)0);
			MODEstatus->when(FL_WHEN_CHANGED);
			MODEstatus->tooltip(_("Left click: change mode\nRight click: configure"));

			cntCW_WPM = new Fl_Counter2(rightof(MODEstatus), Y, Ws2n - Hstatus, Hstatus, "");
			cntCW_WPM->callback(cb_cntCW_WPM);
			cntCW_WPM->minimum(progdefaults.CWlowerlimit);
			cntCW_WPM->maximum(progdefaults.CWupperlimit);
			cntCW_WPM->value(progdefaults.CWspeed);
			cntCW_WPM->tooltip(_("CW transmit WPM"));
			cntCW_WPM->type(1);
			cntCW_WPM->step(1);
			cntCW_WPM->hide();

			btnCW_Default = new Fl_Button(rightof(cntCW_WPM), Y, Hstatus, Hstatus, "*");
			btnCW_Default->callback(cb_btnCW_Default);
			btnCW_Default->tooltip(_("Default WPM"));
			btnCW_Default->hide();

			Status1 = new Fl_Box(rightof(MODEstatus), Y, Ws2n, Hstatus, "");
			Status1->box(FL_DOWN_BOX);
			Status1->color(FL_BACKGROUND2_COLOR);
			Status1->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);

			Status2 = new Fl_Box(rightof(Status1), Y, Wimd, Hstatus, "");
			Status2->box(FL_DOWN_BOX);
			Status2->color(FL_BACKGROUND2_COLOR);
			Status2->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);

			StatusBar = new Fl_Box(
				rightof(Status2), Y,
				progStatus.mainW - bwSqlOnOff - bwAfcOnOff - Wwarn - rightof(Status2) - 2 * pad,// - 60,
				Hstatus, "");
			StatusBar->box(FL_DOWN_BOX);
			StatusBar->color(FL_BACKGROUND2_COLOR);
			StatusBar->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);

			WARNstatus = new Fl_Box(
				rightof(StatusBar) + pad, Y,
				Wwarn, Hstatus, "");
			WARNstatus->box(FL_DIAMOND_DOWN_BOX);
			WARNstatus->color(FL_BACKGROUND_COLOR);
			WARNstatus->labelcolor(FL_RED);
			WARNstatus->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);

			int sql_width = bwSqlOnOff;
#ifdef __APPLE__
			sql_width -= 15; // leave room for resize handle
#endif
			btnAFC = new Fl_Light_Button(
				progStatus.mainW - bwSqlOnOff - bwAfcOnOff,
				Y,
				bwAfcOnOff, Hstatus, "AFC");
			btnAFC->selection_color(progdefaults.AfcColor);
			btnSQL = new Fl_Light_Button(
				progStatus.mainW - bwSqlOnOff,
				Y,
				sql_width, Hstatus, "SQL");
			btnSQL->selection_color(progdefaults.Sql1Color);
			btnAFC->callback(cbAFC, 0);
			btnAFC->value(1);
			btnAFC->tooltip(_("Automatic Frequency Control"));
			btnSQL->callback(cbSQL, 0);
			btnSQL->value(1);
			btnSQL->tooltip(_("Squelch"));

			Fl_Group::current()->resizable(StatusBar);
		hpack->end();

	fl_digi_main->end();
	fl_digi_main->callback(cb_wMain);
	fl_digi_main->resizable(wf);

	struct {
		bool var; const char* label;
	} toggles[] = {
		{ progStatus.DOCKEDSCOPE, DOCKEDSCOPE_MLABEL }
	};
	Fl_Menu_Item* toggle;
	for (size_t i = 0; i < sizeof(toggles)/sizeof(*toggles); i++) {
		if (toggles[i].var && (toggle = getMenuItem(toggles[i].label, alt_menu_))) {
			toggle->set();
			if (toggle->callback()) {
				mnu->value(toggle);
				toggle->do_callback(reinterpret_cast<Fl_Widget*>(mnu));
			}
		}
	}

	make_scopeviewer();
	noop_controls();

	progdefaults.WF_UIwfcarrier =
	progdefaults.WF_UIwfreflevel =
	progdefaults.WF_UIwfampspan =
	progdefaults.WF_UIwfmode =
	progdefaults.WF_UIx1 =
	progdefaults.WF_UIwfshift =
	progdefaults.WF_UIwfdrop = true;
	progdefaults.WF_UIrev =
	progdefaults.WF_UIwfstore =
	progdefaults.WF_UIxmtlock =
	progdefaults.WF_UIqsy = false;
	wf->UI_select(true);

	createConfig();
	if (withnoise)
		grpNoise->show();
	altTabs();

}

//jcoxon

void create_fl_digi_main_dl_fldigi() {
	
	int fnt = fl_font();
	int fsize = fl_size();
	int freqheight = Hentry + 2 * pad;
	int Y = 0;

	fl_font(fnt, freqheight);
	fl_font(fnt, fsize);

//jcoxon
	int Htext = progStatus.mainH - Hwfall - Hmenu - Hstatus - Hmacros - Hqsoframe - 4;
	int minRxHeight = 100;
	int TopFrameHABheight = 80;


	IMAGE_WIDTH = 4000;//progdefaults.HighFreqCutoff;
	Hwfall = progdefaults.wfheight;
	Wwfall = progStatus.mainW - 2 * DEFAULT_SW - 2 * pad;
	w_habCustom = Wwfall;
//jcoxon
//	HAB_height = Hmenu + Hwfall + Hstatus + 4 * pad;
	HAB_height = Hmenu + Hwfall + minRxHeight + TopFrameHABheight + Hstatus + 4 * pad;

	fl_digi_main = new Fl_Double_Window(progStatus.mainW, HAB_height);

		mnuFrame = new Fl_Group(0,0,progStatus.mainW, Hmenu);

			mnu = new Fl_Menu_Bar(0, 0, progStatus.mainW - 200 - pad, Hmenu);
// do some more work on the menu
			for (size_t i = 0; i < sizeof(alt_menu_)/sizeof(alt_menu_[0]); i++) {
// FL_NORMAL_SIZE may have changed; update the menu items
				if (alt_menu_[i].text) {
					alt_menu_[i].labelsize_ = FL_NORMAL_SIZE;
				}
// set the icon label for items with the multi label type
				if (alt_menu_[i].labeltype() == _FL_MULTI_LABEL)
					set_icon_label(&alt_menu_[i]);
			}
			mnu->menu(alt_menu_);

			btnAutoSpot = new Fl_Light_Button(progStatus.mainW - 200 - pad, 0, 50, Hmenu, "Spot");
			btnAutoSpot->selection_color(FL_YELLOW);
			btnAutoSpot->callback(cbAutoSpot, 0);
			btnAutoSpot->deactivate();

			btnRSID = new Fl_Light_Button(progStatus.mainW - 150 - pad, 0, 50, Hmenu, "RxID");
			btnRSID->selection_color(FL_GREEN);
			btnRSID->tooltip("Receive RSID");
			btnRSID->callback(cbRSID, 0);

			btnTxRSID = new Fl_Light_Button(progStatus.mainW - 100 - pad, 0, 50, Hmenu, "TxID");
			btnTxRSID->selection_color(FL_BLUE);
			btnTxRSID->tooltip("Transmit RSID");
			btnTxRSID->callback(cbTxRSID, 0);

			btnTune = new Fl_Light_Button(progStatus.mainW - 50 - pad, 0, 50, Hmenu, "TUNE");
			btnTune->selection_color(FL_RED);
			btnTune->callback(cbTune, 0);

		mnuFrame->resizable(mnu);
		mnuFrame->end();

//jcoxon

		Y = Hmenu + pad;
		
		TopFrameHAB = new Fl_Group(0, Y, progStatus.mainW, TopFrameHABheight);

		{ habFlightXML = new Fl_Choice(10, (Y + Hentry), w_habFlightXML, Hentry, _("Flight"));
		habFlightXML->tooltip(_("Select flight you are tracking"));
		habFlightXML->down_box(FL_BORDER_BOX);
		habFlightXML->align(FL_ALIGN_TOP);
		habFlightXML->when(FL_WHEN_CHANGED);
		habFlightXML->callback(cb_dl_fldigi_select_payload);
		}

		{ habTime = new Fl_Output((rightof(habFlightXML) + 2), (Y + Hentry), w_habTime, Hentry, "Time");
		habTime->tooltip(_("Time"));
		habTime->box(FL_DOWN_BOX);
		habTime->color(FL_BACKGROUND2_COLOR);
		habTime->selection_color(FL_SELECTION_COLOR);
		habTime->labeltype(FL_NORMAL_LABEL);
		habTime->labelfont(0);
		habTime->labelsize(13);
		habTime->labelcolor(FL_FOREGROUND_COLOR);
		habTime->align(FL_ALIGN_TOP); }

		{ habLat = new Fl_Output((rightof(habTime) + 2), (Y + Hentry) , w_habLat, Hentry, "Latitude");
		habLat->tooltip(_("Latitude"));
		habLat->box(FL_DOWN_BOX);
		habLat->color(FL_BACKGROUND2_COLOR);
		habLat->selection_color(FL_SELECTION_COLOR);
		habLat->labeltype(FL_NORMAL_LABEL);
		habLat->labelfont(0);
		habLat->labelsize(13);
		habLat->labelcolor(FL_FOREGROUND_COLOR);
		habLat->align(FL_ALIGN_TOP); }

		{ habLon = new Fl_Output((rightof(habLat) + 2), (Y + Hentry) , w_habLon, Hentry, "Longitude");
		habLon->tooltip(_("Longitude"));
		habLon->box(FL_DOWN_BOX);
		habLon->color(FL_BACKGROUND2_COLOR);
		habLon->selection_color(FL_SELECTION_COLOR);
		habLon->labeltype(FL_NORMAL_LABEL);
		habLon->labelfont(0);
		habLon->labelsize(13);
		habLon->labelcolor(FL_FOREGROUND_COLOR);
		habLon->align(FL_ALIGN_TOP); }

		{ habAlt = new Fl_Output((rightof(habLon) + 2), (Y + Hentry) , w_habAlt, Hentry, "Altitude");
		habAlt->tooltip(_("Altitude"));
		habAlt->box(FL_DOWN_BOX);
		habAlt->color(FL_BACKGROUND2_COLOR);
		habAlt->selection_color(FL_SELECTION_COLOR);
		habAlt->labeltype(FL_NORMAL_LABEL);
		habAlt->labelfont(0);
		habAlt->labelsize(13);
		habAlt->labelcolor(FL_FOREGROUND_COLOR);
		habAlt->align(FL_ALIGN_TOP); }
		
		{ habChecksum = new Fl_Output((rightof(habAlt) + 2), (Y + Hentry) , w_habChecksum, Hentry, "Checksum");
		habChecksum->tooltip(_("Checksum"));
		habChecksum->box(FL_DOWN_BOX);
		habChecksum->color(FL_BACKGROUND2_COLOR);
		habChecksum->selection_color(FL_SELECTION_COLOR);
		habChecksum->labeltype(FL_NORMAL_LABEL);
		habChecksum->labelfont(0);
		habChecksum->labelsize(13);
		habChecksum->labelcolor(FL_FOREGROUND_COLOR);
		habChecksum->align(FL_ALIGN_TOP); }

		{ habBearing = new Fl_Output((rightof(habChecksum) + 2), (Y + Hentry) , w_habBearing, Hentry, "Bearing");
		habBearing->tooltip(_("Bearing from Rx Station to Payload"));
		habBearing->box(FL_DOWN_BOX);
		habBearing->color(FL_BACKGROUND2_COLOR);
		habBearing->selection_color(FL_SELECTION_COLOR);
		habBearing->labeltype(FL_NORMAL_LABEL);
		habBearing->labelfont(0);
		habBearing->labelsize(13);
		habBearing->labelcolor(FL_FOREGROUND_COLOR);
		habBearing->align(FL_ALIGN_TOP); }
		
		{ habDistance = new Fl_Output((rightof(habBearing) + 2), (Y + Hentry) , w_habDistance, Hentry, "Distance");
		habDistance->tooltip(_("Distance from Rx Station to Payload"));
		habDistance->box(FL_DOWN_BOX);
		habDistance->color(FL_BACKGROUND2_COLOR);
		habDistance->selection_color(FL_SELECTION_COLOR);
		habDistance->labeltype(FL_NORMAL_LABEL);
		habDistance->labelfont(0);
		habDistance->labelsize(13);
		habDistance->labelcolor(FL_FOREGROUND_COLOR);
		habDistance->align(FL_ALIGN_TOP); }
	
		{ habTimeSinceLastRx = new Fl_Output(progStatus.mainW - w_habTimeSinceLastRx - 2 * DEFAULT_SW, (Y + Hentry) , w_habTimeSinceLastRx, Hentry, "Time since Rx");
		habTimeSinceLastRx->tooltip(_("Elapsed time since last line of telemetry received"));
		habTimeSinceLastRx->box(FL_DOWN_BOX);
		habTimeSinceLastRx->color(FL_BACKGROUND2_COLOR);
		habTimeSinceLastRx->selection_color(FL_SELECTION_COLOR);
		habTimeSinceLastRx->labeltype(FL_NORMAL_LABEL);
		habTimeSinceLastRx->labelfont(0);
		habTimeSinceLastRx->labelsize(13);
		habTimeSinceLastRx->labelcolor(FL_FOREGROUND_COLOR);
		habTimeSinceLastRx->align(FL_ALIGN_TOP); }

		{ habConfigureButton = new Fl_Button((leftof(habTimeSinceLastRx) - 2 - w_habConfigureButton), (Y + Hentry), w_habConfigureButton, Hentry, "Autoconfigure");
		habConfigureButton->tooltip("Automatically set the fldigi modem settings for the chosen payload.");
		habConfigureButton->labeltype(FL_NORMAL_LABEL);
		habConfigureButton->labelfont(0);
		habConfigureButton->labelsize(13);
		habConfigureButton->when(FL_WHEN_RELEASE);
		habConfigureButton->align(FL_ALIGN_INSIDE);
		habConfigureButton->callback(cb_dl_fldigi_configure_payload);
		}
		
		{ habCustom = new Fl_Output(10, below(habFlightXML) + 4, w_habCustom, Hentry);
		habCustom->tooltip(_("Custom"));
		habCustom->box(FL_DOWN_BOX);
		habCustom->color(FL_BACKGROUND2_COLOR);
		habCustom->selection_color(FL_SELECTION_COLOR);
		habCustom->when(FL_WHEN_RELEASE);}

		Fl_Group::current()->resizable(TopFrameHAB);
		//TopFrameHAB->resizable(TopFrameHAB);
		TopFrameHAB->end();
		
		Y = Hmenu + pad + TopFrameHABheight;
		
		TiledGroup = new Fl_Tile_Check(0, Y, progStatus.mainW, Htext);
			ReceiveText = new FTextRX(0, Y, progStatus.mainW, minRxHeight, "");
			ReceiveText->color(
				fl_rgb_color(
					progdefaults.RxColor.R,
					progdefaults.RxColor.G,
					progdefaults.RxColor.B));
			ReceiveText->setFont(progdefaults.RxFontnbr);
			ReceiveText->setFontSize(progdefaults.RxFontsize);
			ReceiveText->setFontColor(progdefaults.RxFontcolor, FTextBase::RECV);
			ReceiveText->setFontColor(progdefaults.XMITcolor, FTextBase::XMIT);
			ReceiveText->setFontColor(progdefaults.CTRLcolor, FTextBase::CTRL);
			ReceiveText->setFontColor(progdefaults.SKIPcolor, FTextBase::SKIP);
			ReceiveText->setFontColor(progdefaults.ALTRcolor, FTextBase::ALTR);
			int sw = DEFAULT_SW;

			Fl_Box *minbox = new Fl_Box(0,Y + minRxHeight + 1, progStatus.mainW, minRxHeight);
			//Fl_Box *minbox = new Fl_Box(sw,Y + 66, progStatus.mainW-sw, Htext - 66 - 66);
			minbox->hide();

			TiledGroup->resizable(minbox);
			
		TiledGroup->end();
		Fl_Group::current()->resizable(TiledGroup);
//

		Y = Hmenu + pad + TopFrameHABheight + minRxHeight;

		Fl_Pack *wfpack = new Fl_Pack(0, Y, progStatus.mainW, Hwfall);
			wfpack->type(1);
			wf = new waterfall(0, Y, Wwfall, Hwfall);
			wf->end();

			pgrsSquelch = new Progress(
				rightof(wf), Y + pad,
				DEFAULT_SW, Hwfall - 2 * pad,
				"");
			pgrsSquelch->color(FL_BACKGROUND2_COLOR, FL_DARK_GREEN);
			pgrsSquelch->type(Progress::VERTICAL);
			pgrsSquelch->tooltip(_("Detected signal level"));

			sldrSquelch = new Fl_Slider2(
				rightof(pgrsSquelch), Y + pad,
				DEFAULT_SW, Hwfall - 2 * pad,
				"");
			sldrSquelch->minimum(100);
			sldrSquelch->maximum(0);
			sldrSquelch->step(1);
			sldrSquelch->value(progStatus.sldrSquelchValue);
			sldrSquelch->callback((Fl_Callback*)cb_sldrSquelch);
			sldrSquelch->color(FL_INACTIVE_COLOR);
			sldrSquelch->tooltip(_("Squelch level"));
			Fl_Group::current()->resizable(wf);
		wfpack->end();

		Y += (Hwfall + pad);

		Fl_Pack *hpack = new Fl_Pack(0, Y, progStatus.mainW, Hstatus);
			hpack->type(1);
			MODEstatus = new Fl_Button(0, Y, Wmode+30, Hstatus, "");
			MODEstatus->box(FL_DOWN_BOX);
			MODEstatus->color(FL_BACKGROUND2_COLOR);
			MODEstatus->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
			MODEstatus->callback(status_cb, (void *)0);
			MODEstatus->when(FL_WHEN_CHANGED);
			MODEstatus->tooltip(_("Left click: change mode\nRight click: configure"));

			Status1 = new Fl_Box(rightof(MODEstatus), Y, Ws2n, Hstatus, "");
			Status1->box(FL_DOWN_BOX);
			Status1->color(FL_BACKGROUND2_COLOR);
			Status1->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);

			Status2 = new Fl_Box(rightof(Status1), Y, Wimd, Hstatus, "");
			Status2->box(FL_DOWN_BOX);
			Status2->color(FL_BACKGROUND2_COLOR);
			Status2->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);

			StatusBar = new Fl_Box(
				rightof(Status2), Y,
				progStatus.mainW - bwSqlOnOff - bwAfcOnOff - Wwarn - rightof(Status2) - 2 * pad,// - 60,
				Hstatus, "");
			StatusBar->box(FL_DOWN_BOX);
			StatusBar->color(FL_BACKGROUND2_COLOR);
			StatusBar->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);

			WARNstatus = new Fl_Box(
				rightof(StatusBar) + pad, Y,
				Wwarn, Hstatus, "");
			WARNstatus->box(FL_DIAMOND_DOWN_BOX);
			WARNstatus->color(FL_BACKGROUND_COLOR);
			WARNstatus->labelcolor(FL_RED);
			WARNstatus->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);

			int sql_width = bwSqlOnOff;
#ifdef __APPLE__
			sql_width -= 15; // leave room for resize handle
#endif
			btnAFC = new Fl_Light_Button(
				progStatus.mainW - bwSqlOnOff - bwAfcOnOff,
				Y,
				bwAfcOnOff, Hstatus, "AFC");
			btnAFC->selection_color(progdefaults.AfcColor);
			btnSQL = new Fl_Light_Button(
				progStatus.mainW - bwSqlOnOff,
				Y,
				sql_width, Hstatus, "SQL");
			btnSQL->selection_color(progdefaults.Sql1Color);
			
			btnAFC->callback(cbAFC, 0);
			btnAFC->value(1);
			btnAFC->tooltip(_("Automatic Frequency Control"));
			btnSQL->callback(cbSQL, 0);
			btnSQL->value(1);
			btnSQL->tooltip(_("Squelch"));

			Fl_Group::current()->resizable(StatusBar);
		hpack->end();

	fl_digi_main->end();
	fl_digi_main->callback(cb_wMain);
	fl_digi_main->resizable(wf);

	ssdv = new ssdv_rx(320, 240 + 60, _("SSDV RX"));
	ssdv->xclass(PACKAGE_NAME);
	ssdv->hide();

	struct {
		bool var; const char* label;
	} toggles[] = {
		{ progStatus.DOCKEDSCOPE, DOCKEDSCOPE_MLABEL },
	};
	Fl_Menu_Item* toggle;
	for (size_t i = 0; i < sizeof(toggles)/sizeof(*toggles); i++) {
		if (toggles[i].var && (toggle = getMenuItem(toggles[i].label, alt_menu_))) {
			toggle->set();
			if (toggle->callback()) {
				mnu->value(toggle);
				toggle->do_callback(reinterpret_cast<Fl_Widget*>(mnu));
			}
		}
	}

	set_menu_dl_online();

	make_scopeviewer();
	noop_controls();

	progdefaults.WF_UIwfcarrier =
	progdefaults.WF_UIwfreflevel =
	progdefaults.WF_UIwfampspan =
	progdefaults.WF_UIwfmode =
	progdefaults.WF_UIx1 =
	progdefaults.WF_UIwfshift =
	progdefaults.WF_UIwfdrop = true;
	progdefaults.WF_UIrev =
	progdefaults.WF_UIwfstore =
	progdefaults.WF_UIxmtlock =
	progdefaults.WF_UIqsy = false;
	progdefaults.autoextract = true;
	wf->UI_select(true);

	createConfig();
	if (withnoise)
		grpNoise->show();
	altTabs();

}
//

void set_menu_dl_online()
{
	Fl_Menu_Item *i;

	if (bHAB)
	{
		i = getMenuItem(DLFLDIGI_ONLINE_LABEL, alt_menu_);
	}
	else
	{
		i = getMenuItem(DLFLDIGI_ONLINE_LABEL);
	}

	if (progdefaults.dl_online)
	{
		i->set();
	}
	else
	{
		i->clear();
	}
}

void create_fl_digi_main(int argc, char** argv)
{
	if (bWF_only)
		create_fl_digi_main_WF_only();
//jcoxon
	else if (bHAB)
	{
		create_fl_digi_main_dl_fldigi();
		cout << HAB_height << endl;
		}
//
	else
		create_fl_digi_main_primary();

#if defined(__WOE32__)
#  ifndef IDI_ICON
#    define IDI_ICON 101
#  endif
	fl_digi_main->icon((char*)LoadIcon(fl_display, MAKEINTRESOURCE(IDI_ICON)));
#elif !defined(__APPLE__) && USE_X
	make_pixmap(&fldigi_icon_pixmap, fldigi_icon, argc, argv);
	fl_digi_main->icon((char *)fldigi_icon_pixmap);
#endif

	fl_digi_main->xclass(PACKAGE_NAME);

//jcoxon
if (bHAB) {
	fl_digi_main->size_range(WMIN, HAB_height, 0, HAB_height);
}

else {
	fl_digi_main->size_range(WMIN, bWF_only ? WF_only_height : HMIN, 0, bWF_only ? WF_only_height : 0);
	}
//
}

void put_freq(double frequency)
{
	wf->carrier((int)floor(frequency + 0.5));
}

void put_Bandwidth(int bandwidth)
{
	wf->Bandwidth ((int)bandwidth);
}

static void set_metric(double metric)
{
	pgrsSquelch->value(metric);
	if (!progStatus.sqlonoff)
		return;
	if (metric < progStatus.sldrSquelchValue)
		btnSQL->selection_color(progdefaults.Sql1Color);
	else
		btnSQL->selection_color(progdefaults.Sql2Color);
	btnSQL->redraw_label();
}

void display_metric(double metric)
{
	FL_LOCK_D();
	REQ_DROP(set_metric, metric);
	FL_UNLOCK_D();
	FL_AWAKE_D();
}

void put_cwRcvWPM(double wpm)
{
	int U = progdefaults.CWupperlimit;
	int L = progdefaults.CWlowerlimit;
	double dWPM = 100.0*(wpm - L)/(U - L);
	FL_LOCK_D();
	REQ_DROP(static_cast<void (Fl_Progress::*)(float)>(&Fl_Progress::value), prgsCWrcvWPM, dWPM);
	REQ_DROP(static_cast<int (Fl_Value_Output::*)(double)>(&Fl_Value_Output::value), valCWrcvWPM, (int)wpm);
	FL_UNLOCK_D();
	FL_AWAKE_D();
}

void set_scope_mode(Digiscope::scope_mode md)
{
	if (digiscope) {
		digiscope->mode(md);
		REQ(&Fl_Window::size_range, scopeview, SCOPEWIN_MIN_WIDTH, SCOPEWIN_MIN_HEIGHT,
		    0, 0, 0, 0, (md == Digiscope::PHASE || md == Digiscope::XHAIRS));
	}
//	if (wfscope)
//		wfscope->mode(md);
	wf->wfscope->mode(md);
}

void set_scope(double *data, int len, bool autoscale)
{
	if (digiscope)
		digiscope->data(data, len, autoscale);
//	if (wfscope)
//		wfscope->data(data, len, autoscale);
	wf->wfscope->data(data, len, autoscale);
}

void set_phase(double phase, double quality, bool highlight)
{
	if (digiscope)
		digiscope->phase(phase, quality, highlight);
//	if (wfscope)
//		wfscope->phase(phase, quality, highlight);
	wf->wfscope->phase(phase, quality, highlight);
}

void set_rtty(double flo, double fhi, double amp)
{
	if (digiscope)
		digiscope->rtty(flo, fhi, amp);
//	if (wfscope)
//		wfscope->rtty(flo, fhi, amp);
	wf->wfscope->rtty(flo, fhi, amp);
}

void set_video(double *data, int len, bool dir)
{
	if (digiscope)
		digiscope->video(data, len, dir);
//	if (wfscope)
//		wfscope->video(data, len, dir);
	wf->wfscope->video(data, len, dir);
}

void set_zdata(complex *zarray, int len)
{
	if (digiscope)
		digiscope->zdata(zarray, len);
//	if (wfscope)
//		wfscope->zdata(zarray, len);
	wf->wfscope->zdata(zarray, len);
}

static void put_rx_char_flmain(unsigned int data)
{
	ENSURE_THREAD(FLMAIN_TID);

	static unsigned int last = 0;
	const char **asc = ascii;
	trx_mode mode = active_modem->get_mode();

	if (mailclient || mailserver || arqmode)
		asc = ascii2;
	if (mode == MODE_RTTY || mode == MODE_CW)
		asc = ascii;

	int style = FTextBase::RECV;
	if (asc == ascii2 && iscntrl(data))
		style = FTextBase::CTRL;
	if (wf->tmp_carrier())
		style = FTextBase::ALTR;

	if (progdefaults.autoextract == true) rx_extract_add(data);
	speak(data);

	switch (data) {
		case '\n':
			if (last == '\r')
				break;
		case '\r':
			ReceiveText->add('\n', style);
			break;
		default:
			ReceiveText->add(data, style);
	}

	last = data;

	WriteARQ(data);

	string s;
	if (iscntrl(data))
		s = ascii2[data & 0x7F];
	else {
		s += data;
		bool viewer = (mode >= MODE_PSK_FIRST && mode <= MODE_PSK_LAST && dlgViewer && dlgViewer->visible());
		if (progStatus.spot_recv && !viewer)
			spot_recv(data);
	}
	if (Maillogfile)
		Maillogfile->log_to_file(cLogfile::LOG_RX, s);

	if (progStatus.LOGenabled)
		logfile->log_to_file(cLogfile::LOG_RX, s);
}

void put_rx_char(unsigned int data)
{
#if BENCHMARK_MODE
	if (!benchmark.output.empty()) {
		if (unlikely(benchmark.buffer.length() + 16 > benchmark.buffer.capacity()))
			benchmark.buffer.reserve(benchmark.buffer.capacity() + BUFSIZ);
		benchmark.buffer += (char)data;
	}
#else
	REQ(put_rx_char_flmain, data);
#endif
}

static void put_rx_ssdv_flmain(unsigned int data, int lost)
{
	ENSURE_THREAD(FLMAIN_TID);

	if (ssdv)
	{
		ssdv->put_byte(data, lost);
	}
}

void put_rx_ssdv(unsigned int data, int lost)
{
	REQ(put_rx_ssdv_flmain, data, lost);
}

static string strSecText = "";

static void put_sec_char_flmain(char chr)
{
	ENSURE_THREAD(FLMAIN_TID);

	fl_font(FL_HELVETICA, FL_NORMAL_SIZE);
	char s[2] = "W";
	int lc = (int)ceil(fl_width(s));
	int w = StatusBar->w();
	int lw = (int)ceil(fl_width(StatusBar->label()));
	int over = 2 * lc + lw - w;

	if (chr >= ' ' && chr <= 'z') {
		if ( over > 0 )
			strSecText.erase(0, (int)(1.0 * over / lc + 0.5));
		strSecText.append(1, chr);
		StatusBar->label(strSecText.c_str());
		WARNstatus->damage();
	}
}

void put_sec_char(char chr)
{
	REQ(put_sec_char_flmain, chr);
}

static void clear_status_cb(void* arg)
{
	reinterpret_cast<Fl_Box*>(arg)->label("");
}
static void dim_status_cb(void* arg)
{
	reinterpret_cast<Fl_Box*>(arg)->deactivate();
}
static void (*const timeout_action[STATUS_NUM])(void*) = { clear_status_cb, dim_status_cb };

static void put_status_msg(Fl_Box* status, const char* msg, double timeout, status_timeout action)
{
	status->activate();
	status->label(msg);
	if (timeout > 0.0) {
		Fl::remove_timeout(timeout_action[action], status);
		Fl::add_timeout(timeout, timeout_action[action], status);
	}
}

void put_status(const char *msg, double timeout, status_timeout action)
{
	static char m[50];		/* DanielRichman: It looks like this function is meant to be thread safe
					 * (REQ is some form of queue in include/qrunner.h). However, is it really
					 * safe if m is declared static? */
	strncpy(m, msg, sizeof(m));
	m[sizeof(m) - 1] = '\0';

	REQ(put_status_msg, StatusBar, m, timeout, action);
}

void put_Status2(const char *msg, double timeout, status_timeout action)
{
	static char m[60];
	strncpy(m, msg, sizeof(m));
	m[sizeof(m) - 1] = '\0';

	info2msg = msg;

	REQ(put_status_msg, Status2, m, timeout, action);
}

void put_Status1(const char *msg, double timeout, status_timeout action)
{
	static char m[60];
	strncpy(m, msg, sizeof(m));
	m[sizeof(m) - 1] = '\0';

	info1msg = msg;
	if (progStatus.NO_RIGLOG) return;
	REQ(put_status_msg, Status1, m, timeout, action);
}


void put_WARNstatus(double val)
{
	FL_LOCK_D();
	if (val < 0.05)
		WARNstatus->color(FL_BLACK);
    if (val > 0.05)
        WARNstatus->color(FL_DARK_GREEN);
    if (val > 0.9)
        WARNstatus->color(FL_YELLOW);
    if (val > 0.98)
        WARNstatus->color(FL_DARK_RED);
	WARNstatus->redraw();
	FL_UNLOCK_D();
}


void set_CWwpm()
{
	FL_LOCK_D();
	sldrCWxmtWPM->value(progdefaults.CWspeed);
	cntCW_WPM->value(progdefaults.CWspeed);
	FL_UNLOCK_D();
}

void clear_StatusMessages()
{
	FL_LOCK_D();
	StatusBar->label("");
	Status1->label("");
	Status2->label("");
	info1msg = "";
	info2msg = "";
	FL_UNLOCK_D();
	FL_AWAKE_D();
}

void put_MODEstatus(const char* fmt, ...)
{
	static char s[32];
	va_list args;
	va_start(args, fmt);
	vsnprintf(s, sizeof(s), fmt, args);
	va_end(args);

	REQ(static_cast<void (Fl_Button::*)(const char *)>(&Fl_Button::label), MODEstatus, s);
}

void put_MODEstatus(trx_mode mode)
{
	put_MODEstatus("%s", mode_info[mode].sname);
}


void put_rx_data(int *data, int len)
{
 	FHdisp->data(data, len);
}

extern bool macro_idle_on;

char szTestChar[] = "E|I|S|T|M|O|A|V";
int get_tx_char(void)
{
	if (macro_idle_on) return -1;

	if (arq_text_available)
		return arq_get_char();

    if (active_modem == cw_modem && progdefaults.QSKadjust)
        return szTestChar[2 * progdefaults.TestChar];

	int c;
	static int pending = -1;
	if (pending >= 0) {
		c = pending;
		pending = -1;
		return c;
	}

	enum { STATE_CHAR, STATE_CTRL };
	static int state = STATE_CHAR;

	switch (c = TransmitText->nextChar()) {
	case '\n':
		pending = '\n';
		return '\r';
	case '^':
		if (state == STATE_CTRL)
			break;
		state = STATE_CTRL;
		return -1;
	case 'r':
		if (state != STATE_CTRL)
			break;
		REQ_SYNC(&FTextTX::clear_sent, TransmitText);
		state = STATE_CHAR;
		c = 3; // ETX
		break;
	case 'R':
		if (state != STATE_CTRL)
			break;
		state = STATE_CHAR;
		if (TransmitText->eot()) {
			REQ_SYNC(&FTextTX::clear_sent, TransmitText);
			c = 3; // ETX
		} else
			c = -1;
		break;
	case -1:
		break;
	default:
		if (state == STATE_CTRL) {
			state = STATE_CHAR;
			pending = c;
			return '^';
		}
	}

	pending = -1;
	return c;
}

void put_echo_char(unsigned int data)
{
//if (bWF_only) return;
    if (progdefaults.QSKadjust) return;

	static unsigned int last = 0;
	const char **asc = ascii;

	if (mailclient || mailserver || arqmode)
		asc = ascii2;
	if (active_modem->get_mode() == MODE_RTTY ||
		active_modem->get_mode() == MODE_CW)
		asc = ascii;

	if (data == '\r' && last == '\r') // reject multiple CRs
		return;

	last = data;

	int style = FTextBase::XMIT;
	if (asc == ascii2 && iscntrl(data))
		style = FTextBase::CTRL;
	REQ(&FTextBase::addchr, ReceiveText, data, style);

	string s = iscntrl(data) ? ascii2[data & 0x7F] : string(1, data);
	if (Maillogfile)
		Maillogfile->log_to_file(cLogfile::LOG_TX, s);

	if (progStatus.LOGenabled)
		logfile->log_to_file(cLogfile::LOG_TX, s);
}

void resetRTTY() {
	if (active_modem->get_mode() == MODE_RTTY)
		trx_start_modem(active_modem);
}

void resetOLIVIA() {
	if (active_modem->get_mode() == MODE_OLIVIA)
		trx_start_modem(active_modem);
}

void resetCONTESTIA() {
	if (active_modem->get_mode() == MODE_CONTESTIA)
		trx_start_modem(active_modem);
}

void resetTHOR() {
	trx_mode md = active_modem->get_mode();
	if (md == MODE_THOR4 || md == MODE_THOR5 || md == MODE_THOR8 ||
	    md == MODE_THOR11 ||
	    md == MODE_THOR16 || md == MODE_THOR22 )
		trx_start_modem(active_modem);
}

void resetDOMEX() {
	trx_mode md = active_modem->get_mode();
	if (md == MODE_DOMINOEX4 || md == MODE_DOMINOEX5 ||
	    md == MODE_DOMINOEX8 || md == MODE_DOMINOEX11 ||
	    md == MODE_DOMINOEX16 || md == MODE_DOMINOEX22 )
		trx_start_modem(active_modem);
}

void enableMixer(bool on)
{
#if !USE_OSS
	on = false;
#endif

	FL_LOCK_D();
	if (on) {
		progdefaults.EnableMixer = true;
#if USE_OSS
		mixer = new MixerOSS;
#else
		mixer = new MixerBase;
#endif
		try {
			mixer->openMixer(progdefaults.MXdevice.c_str());
		}
		catch (const MixerException& e) {
			put_status(e.what(), 5);
			goto ret;
		}

		mixer->PCMVolume(progdefaults.PCMvolume);
		mixer->setXmtLevel(progStatus.XmtMixer); //valXmtMixer->value());
		mixer->setRcvGain(progStatus.RcvMixer); //valRcvMixer->value());
		if (progdefaults.LineIn == true)
			setMixerInput(1);
		else if (progdefaults.MicIn == true)
			setMixerInput(2);
		else
			setMixerInput(0);
	}else{
		progdefaults.EnableMixer = false;
		if (mixer)
			mixer->closeMixer();
		delete mixer;
                mixer = 0;
	}
ret:
        resetMixerControls();
	FL_UNLOCK_D();
}

void enable_vol_sliders(bool val)
{
if (bWF_only) return;
//jcoxon
if (bHAB) return;
//
        if (MixerFrame->visible()) {
                if (val)
                        return;
		MixerFrame->hide();
		TiledGroup->resize(TiledGroup->x() - MixerFrame->w(), TiledGroup->y(),
				   TiledGroup->w() + MixerFrame->w(), TiledGroup->h());
        }
        else {
                if (!val)
                        return;
		TiledGroup->resize(TiledGroup->x() + MixerFrame->w(), TiledGroup->y(),
				   TiledGroup->w() - MixerFrame->w(), TiledGroup->h());
		MixerFrame->show();
        }
}

void resetMixerControls()
{
if (bWF_only) return;
//jcoxon
if (bHAB) return;
//
    if (progdefaults.EnableMixer) {
	    menuMix->activate();
	    btnLineIn->activate();
	    btnMicIn->activate();
        btnMixer->value(1);
	    valPCMvolume->activate();
    }
    else {
	    menuMix->deactivate();
	    btnLineIn->deactivate();
	    btnMicIn->deactivate();
        btnMixer->value(0);
	    valPCMvolume->deactivate();
    }
    enable_vol_sliders(progdefaults.EnableMixer);
}

void setPCMvolume(double vol)
{
	mixer->PCMVolume(vol);
	progdefaults.PCMvolume = vol;
}

void setMixerInput(int dev)
{
	int n= -1;
	switch (dev) {
		case 0: n = mixer->InputSourceNbr("Vol");
				break;
		case 1: n = mixer->InputSourceNbr("Line");
				break;
		case 2: n = mixer->InputSourceNbr("Mic");
				break;
		default: n = mixer->InputSourceNbr("Vol");
	}
	if (n != -1)
		mixer->SetCurrentInputSource(n);
}

void resetSoundCard()
{
    bool mixer_enabled = progdefaults.EnableMixer;
	enableMixer(false);
	trx_reset();
    if (mixer_enabled)
        enableMixer(true);
}

void setReverse(int rev) {
	active_modem->set_reverse(rev);
}

void start_tx()
{
	if (!(active_modem->get_cap() & modem::CAP_TX))
		return;
	trx_transmit();
	REQ(&waterfall::set_XmtRcvBtn, wf, true);
}

void abort_tx()
{
	if (trx_state == STATE_TUNE) {
		btnTune->value(0);
		btnTune->do_callback();
	}
	else if (trx_state == STATE_TX)
		trx_start_modem(active_modem);
}

void qsy(long long rfc, int fmid)
{
	if (rfc <= 0LL)
		rfc = wf->rfcarrier();

	if (fmid > 0) {
		if (active_modem->freqlocked())
			active_modem->set_freqlock(false);
		else
			active_modem->set_freq(fmid);
		// required for modems that will not change their freq (e.g. mt63)
		int adj = active_modem->get_freq() - fmid;
		if (adj)
			rfc += (wf->USB() ? adj : -adj);
	}

	if (rfc == wf->rfcarrier())
		return;

	if (progdefaults.chkUSERIGCATis)
		REQ(rigCAT_set_qsy, rfc);
	else if (progdefaults.chkUSEMEMMAPis)
		REQ(rigMEM_set_qsy, rfc);
#if USE_HAMLIB
	else if (progdefaults.chkUSEHAMLIBis)
		REQ(hamlib_set_qsy, rfc);
#endif
#if USE_XMLRPC
	else if (progdefaults.chkUSEXMLRPCis)
		REQ(xmlrpc_set_qsy, rfc);
#endif
	else
		LOG_VERBOSE("Ignoring rfcarrier change request (no rig control)");
}

map<string, qrg_mode_t> qrg_marks;
qrg_mode_t last_marked_qrg;

void note_qrg(bool no_dup, const char* prefix, const char* suffix, trx_mode mode, long long rfc, int afreq)
{
	qrg_mode_t m;
	m.rfcarrier = (rfc ? rfc : wf->rfcarrier());
	m.carrier = (afreq ? afreq : active_modem->get_freq());
	m.mode = (mode < NUM_MODES ? mode : active_modem->get_mode());
	if (no_dup && last_marked_qrg == m)
		return;
	last_marked_qrg = m;

	char buf[64];

	time_t t = time(NULL);
	struct tm tm;
	gmtime_r(&t, &tm);
	size_t r1;
	if ((r1 = strftime(buf, sizeof(buf), "<<%Y-%m-%dT%H:%MZ ", &tm)) == 0)
		return;

	size_t r2;
	if (m.rfcarrier)
		r2 = snprintf(buf+r1, sizeof(buf)-r1, "%s @ %lld%c%04d>>",
			     mode_info[m.mode].name, m.rfcarrier, (wf->USB() ? '+' : '-'), m.carrier);
	else
		r2 = snprintf(buf+r1, sizeof(buf)-r1, "%s @ %04d>>", mode_info[m.mode].name, m.carrier);
	if (r2 >= sizeof(buf)-r1)
		return;

	qrg_marks[buf] = m;
	if (prefix && *prefix)
		ReceiveText->add(prefix);
	ReceiveText->add(buf, FTextBase::QSY);
	ReceiveText->mark();
	if (suffix && *suffix)
		ReceiveText->add(suffix);
}

void xmtrcv_selection_color()
{
	wf->xmtrcv_selection_color(progdefaults.XmtColor);
	wf->redraw();
}

void rev_selection_color()
{
	wf->reverse_selection_color(progdefaults.RevColor);
	wf->redraw();
}

void xmtlock_selection_color()
{
	wf->xmtlock_selection_color(progdefaults.LkColor);
	wf->redraw();
}

void sql_selection_color()
{
	btnSQL->selection_color(progdefaults.Sql1Color);
	btnSQL->redraw();
}

void afc_selection_color()
{
	btnAFC->selection_color(progdefaults.AfcColor);
	btnAFC->redraw();
}

void rxid_selection_color()
{
	btnRSID->selection_color(progdefaults.RxIDColor);
	btnRSID->redraw();
}

void txid_selection_color()
{
	btnTxRSID->selection_color(progdefaults.TxIDColor);
	btnTxRSID->redraw();
}

void tune_selection_color()
{
	btnTune->selection_color(progdefaults.TuneColor);
	btnTune->redraw();
}

void spot_selection_color()
{
	btnAutoSpot->selection_color(progdefaults.SpotColor);
	btnAutoSpot->redraw();
}

// Olivia
void set_olivia_bw(int bw)
{
	int i;
	if (bw == 125)
		i = 0;
	else if (bw == 250)
		i = 1;
	else if (bw == 500)
		i = 2;
	else if (bw == 1000)
		i = 3;
	else
		i = 4;
	bool changed = progdefaults.changed;
	mnuOlivia_Bandwidth->value(i);
	mnuOlivia_Bandwidth->do_callback();
	progdefaults.changed = changed;
}

void set_olivia_tones(int tones)
{
	unsigned i = 0;
	while (tones >>= 1)
		i++;
	bool changed = progdefaults.changed;
	mnuOlivia_Tones->value(i - 1);
	mnuOlivia_Tones->do_callback();
	progdefaults.changed = changed;
}

//Contestia
void set_contestia_bw(int bw)
{
	int i;
	if (bw == 125)
		i = 0;
	else if (bw == 250)
		i = 1;
	else if (bw == 500)
		i = 2;
	else if (bw == 1000)
		i = 3;
	else
		i = 4;
	bool changed = progdefaults.changed;
	mnuContestia_Bandwidth->value(i);
	mnuContestia_Bandwidth->do_callback();
	progdefaults.changed = changed;
}

void set_contestia_tones(int tones)
{
	unsigned i = 0;
	while (tones >>= 1)
		i++;
	bool changed = progdefaults.changed;
	mnuContestia_Tones->value(i - 1);
	mnuContestia_Tones->do_callback();
	progdefaults.changed = changed;
}


void set_rtty_shift(int shift)
{
	if (shift < selCustomShift->minimum() || shift > selCustomShift->maximum())
		return;

	const int shifts[] = { 23, 85, 160, 170, 182, 200, 240, 350, 425, 850 };
	size_t i;
	for (i = 0; i < sizeof(shifts)/sizeof(*shifts); i++)
		if (shifts[i] == shift)
			break;
	selShift->value(i);
	selShift->do_callback();
	if (i == sizeof(shifts)/sizeof(*shifts)) {
		selCustomShift->value(shift);
		selCustomShift->do_callback();
	}
}

void set_rtty_baud(float baud)
{
	const float bauds[] = {
		45.0f, 45.45f, 50.0f, 56.0f, 75.0f,
		100.0f, 110.0f, 150.0f, 200.0f, 300.0f
	};
	for (size_t i = 0; i < sizeof(bauds)/sizeof(*bauds); i++) {
		if (bauds[i] == baud) {
			selBaud->value(i);
			selBaud->do_callback();
			break;
		}
	}
}

void set_rtty_bits(int bits)
{
	const int bits_[] = { 5, 7, 8 };
	for (size_t i = 0; i < sizeof(bits_)/sizeof(*bits_); i++) {
		if (bits_[i] == bits) {
			selBits->value(i);
			selBits->do_callback();
			break;
		}
	}
}


