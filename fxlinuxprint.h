/*********************************************************************************
Fuji Xerox Printer Driver for Linux
(C) Fuji Xerox Co.,Ltd. 2006

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
***********************************************************************************/

//-------- macros ------------------------------
#define		numof(x)	(int)(sizeof(x)/sizeof(*x))

//-------- definitions -------------------------
#define		PDF_FILE_NAME  	"/tmp/tmp_fxpdfXXXXXX"

#define		MAX_STR_SIZE		64
#define		MAX_FILE_NAME_SIZE	128
#define		MAX_DECODE_BUF_SIZE	256
#define		READ_DATA_SIZE		(1024*256)
#define		PJL_DATA_SIZE		(1024*1)

#define		PARMS				0666

#define		KEY_OUTPUTMODE		"FXOutputMode"
#define		KEY_QUALITY			"Quality2"

#define		PickOne				1
#define		Integer				2
#define		String				3

#define		RES600				600
#define		RES1200				1200

//-------- memory decralations -----------------
typedef	struct _fx_option_t {
	unsigned int	size;
	int				paper;
	int				layout;
	int				print_mode;
	int				resolution;
	int				duplex;
	int				media_type;
	int				input_slot;
	int				color_mode;
	int				copies;
	int				collate;
	int				job_type;
	int				staple;
	int				punch;
	int				banner;
	char			job_name[128];
	char			hold_key[128];
	char			user_name[128];
	char			bn_user[128];
} FX_OPTION_T, *LPFX_OPTION_T;

//-------- table decralations ------------------
typedef	struct _ppd_option_choice {
	const int	num;
	const char*	key;
	const char*	pjl;
} FX_PPD_OPT_CHOICE, *LPFX_PPD_OPT_CHOICE;

typedef	struct _ppd_option_list {
	const char*			ppd_key;
	int					mode;
	LPFX_PPD_OPT_CHOICE	ppd_options;
	int					num_options;
	void*				p_value;
} FX_PPD_OPT_LIST, *LPFX_PPD_OPT_LIST;

//-------- PJL decralations --------------------
const char PJLHeader[]			= "\x1b%-12345X@PJL\x0a";
const char PJLTrailer[]			= "\x1b%-12345X";
const char PJLCommentBegin[]	= "@PJL COMMENT begin\x0a";
const char PJLCommentVersion[]	= "@PJL COMMENT Version:4.00\x0a";

const char PJLSetRenderMode[]	= "@PJL SET RENDERMODE=";
const char PJLSetPaperSize[]	= "@PJL SET PAPER=";
const char PJLSetCollate[]		= "@PJL SET COLLATE=";
const char PJLSetCopies[]		= "@PJL SET COPIES=";
const char PJLSetQty[]			= "@PJL SET QTY=";
const char PJLSetJobAttribute[]	= "@PJL SET JOBATTR=";
const char PJLSetDuplex[]		= "@PJL SET DUPLEX=";
const char PJLSetBinding[]		= "@PJL SET BINDING=";
const char PJLSetAutoSelectOn[]	= "@PJL SET AUTOSELECT=ON\x0a";
const char PJLSetAutoSelectOff[]	= "@PJL SET AUTOSELECT=OFF\x0a";
const char PJLSetMediaSource[]	= "@PJL SET MEDIASOURCE=";
const char PJLSetMediaType[]	= "@PJL SET MEDIATYPE=";
const char PJLSetOutMain[]		= "@PJL SET OUTBIN=MAINTRAY\x0a";
const char PJLSetOutBin100[]	= "@PJL SET OUTBIN=OUTBIN100\x0a";
const char PJLSetTimeOut[]		= "@PJL SET TIMEOUT=0\x0a";
const char PJLSetHold[]			= "@PJL SET HOLD=";
const char PJLSetUserName[]		= "@PJL SET USERNAME=";
const char PJLSetJobName[]		= "@PJL SET JOBNAME=";
const char PJLSetHoldType[]		= "@PJL SET HOLDTYPE=PRIVATE\x0a";
const char PJLSetHoldKey[]		= "@PJL SET HOLDKEY=";
const char PJLSetFinishOn[]		= "@PJL SET FINISH=ON\x0a";
const char PJLSetStaple[]		= "@PJL SET STAPLE=";
const char PJLSetPunch[]		= "@PJL SET PUNCH=LEFT\x0a";
const char PJLEnterLanguage[]	= "@PJL ENTER LANGUAGE=PDF\x0a";

const char PJLAttributeFstp[]	= "\"@FSTP=0\"\x0a";
const char PJLAttributeLstp[]	= "\"@LSTP=0\"\x0a";
const char PJLAttributeLout[]	= "\"@LOUT=";
const char PJLAttributeZoom[]	= "\"@ZOOM=100\"\x0a";
const char PJLAttributeFsize[]	= "\"@FSIZ=";
const char PJLAttributeAnnt[]   = "\"@ANNT=OFF\"\x0a";
const char PJLAttributeDipq[]	= "\"@DIPQ=";
const char PJLAttributePunch[]	= "\"@PNHN=TWO\"\x0a";
const char PJLAttributeBanr[]	= "\"@BANR=START\"\x0a";
const char PJLAttributeJoau[]	= "\"@JOAU=@";

//-------- prototype decralations --------------
int fxGetPrintOptions (int argc, const char* argv[], LPFX_OPTION_T opt);
int fxMakePJL (char* pjl, LPFX_OPTION_T opt);
int fxExecFilter (char* arg[]);
