// ----------------------------------------------------------------------------
// logger.cxx Remote Log Interface for fldigi
//
// Copyright 2006-2009 W1HKJ, Dave Freese
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
#include <sys/stat.h>
#if !defined(__WOE32__) && !defined(__APPLE__)
#  include <sys/ipc.h>
#  include <sys/msg.h>
#endif
#include <errno.h>
#include <string>

#include "logger.h"
#include "lgbook.h"
#include "main.h"
#include "modem.h"
#include "waterfall.h"
#include "fl_digi.h"
#include "trx.h"
#include "debug.h"
#include "macros.h"
#include "status.h"
#include "spot.h"
#include "adif_io.h"

#include "logsupport.h"

#include <FL/fl_ask.H>

using namespace std;

//---------------------------------------------------------------------
const char *logmode;
char logdate[32];
char logtime[32];

static string log_msg;
static string errmsg;
static string notes;

//=============================================================================

#if defined(__WOE32__) || defined(__APPLE__)

static string adif;

void writeADIF () {
// open the adif file
	FILE *adiFile;
	string sfname;

    sfname = TempDir;
    sfname.append("log.adif");
	adiFile = fopen (sfname.c_str(), "a");
	if (adiFile) {
// write the current record to the file  
		fprintf(adiFile,"%s<EOR>\n", adif.c_str());
		fclose (adiFile);
	}
}

void putadif(int num, const char *s)
{
        char tempstr[100];
        int slen = strlen(s);
        if (slen > fields[num].size) slen = fields[num].size;
        int n = snprintf(tempstr, sizeof(tempstr), "<%s:%d>", fields[num].name, slen);
        if (n == -1) {
		LOG_PERROR("snprintf");
                return;
        }
        memcpy(tempstr + n, s, slen);
        tempstr[n + slen] = '\0';
        adif.append(tempstr);
}

void submit_ADIF(void)
{
	adif.erase();
	
	putadif(QSO_DATE, inpDate_log->value());//logdate); 
	putadif(TIME_ON, inpTimeOn_log->value());
	putadif(TIME_OFF, inpTimeOff_log->value());
	putadif(CALL, inpCall_log->value());
	putadif(FREQ, inpFreq_log->value());
	putadif(MODE, logmode);
	putadif(RST_SENT, inpRstS_log->value());
	putadif(RST_RCVD, inpRstR_log->value());
	putadif(TX_PWR, inpTX_pwr_log->value());
	putadif(NAME, inpName_log->value());
	putadif(QTH, inpQth_log->value());
	putadif(STATE, inpState_log->value());
	putadif(VE_PROV, inpVE_Prov_log->value());
	putadif(COUNTRY, inpCountry_log->value());
	putadif(GRIDSQUARE, inpLoc_log->value());
	putadif(STX, inpSerNoOut_log->value());
	putadif(SRX, inpSerNoIn_log->value());
	putadif(XCHG1, inpXchgIn_log->value());
	putadif(MYXCHG, inpMyXchg_log->value());
	notes = inpNotes_log->value();
	for (size_t i = 0; i < notes.length(); i++)
	    if (notes[i] == '\n') notes[i] = ';';
	putadif(NOTES, notes.c_str());
// these fields will always be blank unless they are added to the main
// QSO log area.
	putadif(IOTA, inpIOTA_log->value());
	putadif(DXCC, inpDXCC_log->value());
	putadif(QSLRDATE, inpQSLrcvddate_log->value());
	putadif(QSLSDATE, inpQSLsentdate_log->value());

	writeADIF();
}

#endif

//---------------------------------------------------------------------
// the following IPC message is compatible with xlog remote data spec.
//---------------------------------------------------------------------

#if !defined(__WOE32__) && !defined(__APPLE__)

#define addtomsg(x, y)	log_msg = log_msg + (x) + (y) + LOG_MSEPARATOR

static void send_IPC_log(void)
{
	msgtype msgbuf;
	const char   LOG_MSEPARATOR[2] = {1,0};
	int msqid, len;
	
	log_msg = "";
	log_msg = log_msg + "program:"	+ PACKAGE_NAME + " v " 	+ PACKAGE_VERSION + LOG_MSEPARATOR;
	addtomsg("version:",	LOG_MVERSION);
	addtomsg("date:",		logdate);
	addtomsg("time:", 		inpTimeOn_log->value());
	addtomsg("endtime:", 	inpTimeOff_log->value());
	addtomsg("call:",		inpCall_log->value());
	addtomsg("mhz:",		inpFreq_log->value());
	addtomsg("mode:",		logmode);
	addtomsg("tx:",			inpRstS_log->value());
	addtomsg("rx:",			inpRstR_log->value());
	addtomsg("name:",		inpName_log->value());
	addtomsg("qth:",		inpQth_log->value());
	addtomsg("state:",		inpState_log->value());
	addtomsg("province:",	inpVE_Prov_log->value());
	addtomsg("country:",	inpCountry_log->value());
	addtomsg("locator:",	inpLoc_log->value());
	addtomsg("serialout:",	inpSerNoOut_log->value());
	addtomsg("serialin:",	inpSerNoIn_log->value());
	addtomsg("free1:",		inpXchgIn_log->value());
	notes = inpNotes_log->value();
	for (size_t i = 0; i < notes.length(); i++)
	    if (notes[i] == '\n') notes[i] = ';';
	addtomsg("notes:",		notes.c_str());
	addtomsg("power:",		inpTX_pwr_log->value());
	
	strncpy(msgbuf.mtext, log_msg.c_str(), sizeof(msgbuf.mtext));
	msgbuf.mtext[sizeof(msgbuf.mtext) - 1] = '\0';

	if ((msqid = msgget(LOG_MKEY, 0666 | IPC_CREAT)) == -1) {
		LOG_PERROR("msgget");
		return;
	}
	msgbuf.mtype = LOG_MTYPE;
	len = strlen(msgbuf.mtext) + 1;
	if (msgsnd(msqid, &msgbuf, len, IPC_NOWAIT) < 0)
		LOG_PERROR("msgsnd");
}

#endif

//---------------------------------------------------------------------

void submit_log(void)
{
	if (progStatus.spot_log)
		spot_log(inpCall->value(), inpLoc->value());

	struct tm *tm;
	time_t t;

	time(&t);
        tm = gmtime(&t);
		strftime(logdate, sizeof(logdate), "%d %b %Y", tm);
		strftime(logtime, sizeof(logtime), "%H%M", tm);
	logmode = mode_info[active_modem->get_mode()].adif_name;

	AddRecord();

#if !defined(__WOE32__) && !defined(__APPLE__)
	send_IPC_log();
#else
	submit_ADIF();
#endif

}

