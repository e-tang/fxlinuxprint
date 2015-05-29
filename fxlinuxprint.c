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
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#define __USE_XOPEN_EXTENDED
#include <signal.h>
#include <wait.h>
#include <unistd.h>
#include <cups/cups.h>
#include <cups/ppd.h>
#include "fxlinuxprint.h"
#include "codec.h"

// paper size
enum {
	PAPER_AUTO = 0,
	PAPER_A4,
	PAPER_A3,
	PAPER_A5,
	PAPER_B4,
	PAPER_B5,
	PAPER_LETTER,
	PAPER_LEGAL,
	PAPER_LEDGER
};

FX_PPD_OPT_CHOICE	paper_size_list[] =
{
	{ PAPER_AUTO,	"Auto",		"AUTO\x0a" },
	{ PAPER_A4,		"A4",		"A4\x0a" },
	{ PAPER_A3,		"A3",		"A3\x0a" },
	{ PAPER_A5,		"A5",		"A5\x0a"},
	{ PAPER_B4,		"B4",		"JISB4\x0a" },
	{ PAPER_B5,		"B5",		"JISB5\x0a" },
	{ PAPER_LETTER,	"Letter",	"LETTER\x0a" },
	{ PAPER_LEGAL,	"Legal",	"LEGAL\x0a" },
	{ PAPER_LEDGER,	"Ledger",	"LEDGER\x0a" },
};

// layout
enum {
	LOUT_AUTO = 0,
	LOUT_LEFTUP,
	LOUT_CENTER,
	LOUT_BOOKLET,
	LOUT_N2,
	LOUT_N4
};

FX_PPD_OPT_CHOICE	layout_list[] =
{
	{ LOUT_AUTO,	"Auto",		"AUTOSCALING\"\x0a" },
	{ LOUT_LEFTUP,	"TopLeft",	"NOSCALING\"\x0a" },
	{ LOUT_CENTER,	"Center",	"CENTER\"\x0a" },
	{ LOUT_BOOKLET,	"Booklet",	"BOOKLET\"\x0a" },
	{ LOUT_N2,		"2up",		"N2\"\x0a" },
	{ LOUT_N4,		"4up",		"N4\"\x0a" },
};

// print mode
enum {
	PM_STANDARD = 0,
	PM_QUALITY2,
	PM_SPEED
};

FX_PPD_OPT_CHOICE	print_mode_list[] =
{
	{ PM_STANDARD,	"Standard",	"STANDARD\"\x0a" },
	{ PM_QUALITY2,	"Quality2",	"QUALITY2\"\x0a" },
	{ PM_SPEED,		"Speed",	"SPEED\"\x0a" },
};

// duplex
enum {
	DUPLEX_OFF = 0,
	DUPLEX_ON_SHORT,
	DUPLEX_ON_LONG
};

FX_PPD_OPT_CHOICE	duplex_list[] =
{
	{ DUPLEX_OFF,		"None",				"OFF\x0a" },
	{ DUPLEX_ON_SHORT,	"DuplexTumble",		"ON\x0a" },
	{ DUPLEX_ON_LONG,	"DuplexNoTumble",	"ON\x0a" },
};

enum {
	BINDING_OFF = 0,
	BINDING_SHORT,
	BINDING_LONG
};

FX_PPD_OPT_CHOICE	binding_list[] =
{
	{ BINDING_OFF,		"",	"" },
	{ BINDING_SHORT,	"",	"SHORTEDGE\x0a" },
	{ BINDING_LONG,		"",	"LONGEDGE\x0a" },
};

// media type
enum {
	MEDIA_NORMAL = 0,
	MEDIA_PRINTER
};

FX_PPD_OPT_CHOICE	media_type_list[] =
{
	{ MEDIA_NORMAL,		"Plain",	"NORMAL\x0a" },
	{ MEDIA_PRINTER,	"Printer",	"" },
};

// input slot
enum {
	TRAY_AUTO = 0,
	TRAY_1,
	TRAY_2,
	TRAY_3,
	TRAY_4,
	TRAY_5
};

FX_PPD_OPT_CHOICE	input_slot_list[] =
{
	{ TRAY_AUTO,		"Auto",		"" },
	{ TRAY_1,			"Tray1",	"TRAY1\x0a" },
	{ TRAY_2,			"Tray2",	"TRAY2\x0a" },
	{ TRAY_3,			"Tray3",	"TRAY3\x0a" },
	{ TRAY_4,			"Tray4",	"TRAY4\x0a" },
	{ TRAY_5,			"Tray5",	"TRAY5\x0a" },
};

// color mode
enum {
	COLOR_AUTO = 0,
	COLOR_GRAY
};

FX_PPD_OPT_CHOICE	color_mode_list[] =
{
	{ COLOR_AUTO,		"Auto",		"AUTOCOLOR\x0a" },
	{ COLOR_GRAY,		"Black",	"GRAYSCALE\x0a" },
};

// collate
enum {
	COLLATE_OFF = 0,
	COLLATE_ON
};

FX_PPD_OPT_CHOICE	collate_list[] =
{
	{ COLLATE_OFF,		"False",	"OFF\x0a" },
	{ COLLATE_ON,		"True",		"COLLATENORMAL\x0a" },
};

// job type
enum {
	JOB_NORMAL = 0,
	JOB_SECURITY,
	JOB_SAMPLE
};

FX_PPD_OPT_CHOICE	job_type_list[] =
{
	{ JOB_NORMAL,		"Normal",		"" },
	{ JOB_SECURITY,		"Secure",		"STORE\x0a" },
	{ JOB_SAMPLE,		"Sample",		"PROOF\x0a" },
};

// staple
enum {
	STAPLE_OFF = 0,
	STAPLE_TOPLEFT,
	STAPLE_LEFTDUAL,
	STAPLE_TOPDUAL
};

FX_PPD_OPT_CHOICE	staple_list[] =
{
	{ STAPLE_OFF,		"None",				"" },
	{ STAPLE_TOPLEFT,	"UpperLeftSingle",	"TOPLEFT\x0a" },
	{ STAPLE_LEFTDUAL,	"LeftDouble",		"LEFTDUAL\x0a" },
	{ STAPLE_TOPDUAL,	"TopDouble",		"TOPDUAL\x0a" },
};

// punch
enum {
	PUNCH_OFF = 0,
	PUNCH_ON
};

FX_PPD_OPT_CHOICE	punch_list[] =
{
	{ PUNCH_OFF,	"Off",	"" },
	{ PUNCH_ON,		"On",	"" },
};

// banner
enum {
	BANNER_OFF = 0,
	BANNER_ON
};

FX_PPD_OPT_CHOICE	banner_list[] =
{
	{ BANNER_OFF,	"False",	"" },
	{ BANNER_ON,	"True",		"" },
};

FX_OPTION_T	printOption;
FX_OPTION_T	printOptionDefault = 
{
	0,
	PAPER_A4,
	LOUT_LEFTUP,
	PM_STANDARD,
	RES600,
	DUPLEX_OFF,
	MEDIA_NORMAL,
	TRAY_AUTO,
	COLOR_AUTO,
	1,
	COLLATE_OFF,
	JOB_NORMAL,
	STAPLE_OFF,
	PUNCH_OFF,
	BANNER_OFF,
	"(unknown)",
	"",
	"(unknown)",
	"(unknown)"
};

FX_PPD_OPT_LIST		option_list[] =
{
	{"PageSize",		PickOne, 	&paper_size_list[0],	numof(paper_size_list), (void*)&printOption.paper },
	{"FXLayout",		PickOne,	&layout_list[0],		numof(layout_list),		(void*)&printOption.layout },
	{"FXOutputMode",	PickOne,	&print_mode_list[0],	numof(print_mode_list),	(void*)&printOption.print_mode},
	{"Duplex",			PickOne,	&duplex_list[0],		numof(duplex_list),		(void*)&printOption.duplex},
	{"FXMediaType",		PickOne,	&media_type_list[0],	numof(media_type_list),	(void*)&printOption.media_type},
	{"InputSlot",		PickOne,	&input_slot_list[0],	numof(input_slot_list),	(void*)&printOption.input_slot},
	{"FXColorMode",		PickOne,	&color_mode_list[0],	numof(color_mode_list),	(void*)&printOption.color_mode},
	{"FXCollate",		PickOne,	&collate_list[0],		numof(collate_list),	(void*)&printOption.collate},
	{"FXStaple",		PickOne,	&staple_list[0],		numof(staple_list),		(void*)&printOption.staple},
	{"FXPunch",			PickOne,	&punch_list[0],			numof(punch_list),		(void*)&printOption.punch},
	{"FXJobType",		PickOne,	&job_type_list[0],		numof(job_type_list),	(void*)&printOption.job_type},
//	{"FXBanner",		PickOne,	&banner_list[0],		numof(banner_list),		(void*)&printOption.banner},
	{"FXCopies",		Integer,	0,						0, 						(void*)&printOption.copies },
	{"FXHoldKey",		String,		0,						0,						(void*)&printOption.hold_key },
	{"FXUserName",		String,		0,						0,						(void*)&printOption.user_name},
//	{"FXBnUser",		String,		0,						0,						(void*)&printOption.bn_user},
};

char* argPS2PDF[] =
{
	"ps2pdf",
	"-r",
	"-",
	"",
	NULL
};

//==============================================
// main ( )
//==============================================
int main (int argc, const char* argv[])
{
	static char 	ch[READ_DATA_SIZE+1];
	static char 	chPJL[PJL_DATA_SIZE+1];
	char			pdf_file_name[MAX_FILE_NAME_SIZE];
	int 			fd_in;
	unsigned int 	len = 0;
	struct stat 	stbuf;

	setbuf (stderr, NULL);

	// set default
	memcpy ((char*)&printOption, (char*)&printOptionDefault, sizeof(FX_OPTION_T));
	
	// get print information.
	fxGetPrintOptions (argc, argv, &printOption);

	strcpy(pdf_file_name, PDF_FILE_NAME);
	mkstemp(pdf_file_name);

#ifdef	__PS2PDF__
	// convert PS to PDF (GohstScript) : stdin -> file
	{
	char str[MAX_STR_SIZE];
	sprintf (str, "%s%d", argPS2PDF[1], printOption.resolution);
	argPS2PDF[1] = str;
	argPS2PDF[3] = pdf_file_name;

	if (fxExecFilter (argPS2PDF) < 0) {
		return (-2);
	}
	}
#else
	// convert PDF to PDF : stdin -> file
	{
	int	fd_out;
	if ((fd_out = open (pdf_file_name, O_RDWR)) < 0) {
		fprintf (stderr, "ERROR: file not opend. %s", pdf_file_name);
		return (-2);
	}
	while ((len = read (0, ch, READ_DATA_SIZE)) > 0) {
		write (fd_out, ch, len);
	}
	close (fd_out);
	}
#endif	//__PS2PDF__

	// get PDF file size
	stat (pdf_file_name, &stbuf);
	printOption.size = stbuf.st_size;

	// convert PDF to PJL+PDF : file -> stdout
	if ((fd_in = open (pdf_file_name, O_RDONLY)) < 0) {
		fprintf (stderr, "ERROR: file not opend. %s", pdf_file_name);
		return (-2);
	}
	fxMakePJL (chPJL, &printOption);
	write (1, chPJL, strlen (chPJL));

	while ((len = read (fd_in, ch, READ_DATA_SIZE)) > 0) {
		write (1, ch, len);
	}
	write (1, PJLTrailer, strlen (PJLTrailer));
	close (fd_in);

	// delete temp file
	unlink (pdf_file_name);

	return (0);
}

//==============================================
// fxGetPrintOptions ( )
//==============================================
int fxGetPrintOptions (int argc, const char* argv[], LPFX_OPTION_T opt)
{
	const char			*file_name;
	int 				num_dests;
	int 				i;
	int 				j;
	int					num_options;
	int					myalloc;
	cups_option_t		*p_options;
	cups_dest_t			*dest;
	cups_dest_t			*dests;
	ppd_choice_t		*p_choice;
	ppd_file_t			*p_ppd;
	LPFX_PPD_OPT_CHOICE	p_option;

	// get destination printer
	num_dests = cupsGetDests (&dests);
	dest = cupsGetDest (NULL, NULL, num_dests, dests);
	file_name = cupsGetPPD (dest->name);

	cupsFreeDests (num_dests, dests);

	// get options
	strcpy ( opt->job_name, argv[3]);
	num_options = cupsParseOptions (argv[5], 0, &p_options);

	if( (p_ppd = ppdOpenFile(file_name)) != NULL ) {
		ppdMarkDefaults (p_ppd);
		cupsMarkOptions (p_ppd, num_options, p_options);
		for (i=0; i < numof(option_list); i++) {
			myalloc = 0;
			p_choice = NULL;
			// find marked choice
			if ((p_choice = ppdFindMarkedChoice (p_ppd, option_list[i].ppd_key)) == NULL) {
				// ppd key != argument(argv[5]) key
				for (j=0; j < num_options; j++) {
					if (strcmp(option_list[i].ppd_key, p_options[j].name)==0) {
						if ((p_choice = (ppd_choice_t*) malloc (sizeof(ppd_choice_t))) != NULL) {
							strcpy(p_choice->choice, p_options[j].value);
							myalloc = 1;
						}
						break;
					}
				}
			}

			// set value
			//   optin_list[i].ppd_key : option key name (string)
			//   p_choice->choice : option value (string)
			if (p_choice != NULL) {
				p_option = option_list[i].ppd_options;
				switch (option_list[i].mode) {
				case PickOne:
					for (j=0; j < option_list[i].num_options; j++, p_option++) {
						if (strcmp(p_option->key, p_choice->choice) == 0) {
							*((int*)option_list[i].p_value) = p_option->num;
							if (strcmp(option_list[i].ppd_key, KEY_OUTPUTMODE) == 0) {
								if (strcmp(p_choice->choice, KEY_QUALITY) == 0) {
									opt->resolution = RES1200;
								} else {
									opt->resolution = RES600;
								}
							}
						}
					}
					break;
				case Integer:
					*((int*)option_list[i].p_value) = atoi (p_choice->choice);
					break;
				case String:
					strcpy ((char*)option_list[i].p_value, (char*)p_choice->choice);
					break;
				default:
					break;
				}
				if (myalloc) {
					free (p_choice);
				}
			}
		}
		ppdClose(p_ppd);
	}
	cupsFreeOptions (num_options, p_options);

	// delete temp file
	unlink (file_name);


	return (0);
}

//==============================================
// fxMakePJL
//==============================================
int fxMakePJL (char* pjl, LPFX_OPTION_T opt)
{
	char str[MAX_STR_SIZE];
    char decode_buff[MAX_DECODE_BUF_SIZE];
    int  decode_len;

	// const
	if (opt->job_type == JOB_SAMPLE) {
		opt->collate = COLLATE_ON;
	}
	if (opt->layout == LOUT_N2 || opt->layout == LOUT_N4) {
		opt->paper = PAPER_A4;
	} else if (opt->layout == LOUT_BOOKLET) {
		opt->duplex = DUPLEX_ON_LONG;
		opt->media_type = MEDIA_NORMAL;
		if (opt->paper != PAPER_A4) {
			opt->paper = PAPER_AUTO;
		}
	}
//	if (opt->staple != STAPLE_OFF || opt->punch != PUNCH_OFF) {
//		opt->paper = PAPER_AUTO;
//	}

	// PJL
	strcpy (pjl, PJLHeader);
	strcat (pjl, PJLCommentBegin);
	strcat (pjl, PJLCommentVersion);
	sprintf (str, "%s%s", PJLSetRenderMode, color_mode_list[opt->color_mode].pjl);
	strcat (pjl, str);
	strcat (pjl, PJLSetJobAttribute);
	sprintf (str, "%s%s", PJLAttributeDipq, print_mode_list[opt->print_mode].pjl);
	strcat (pjl, str);
	sprintf (str, "%s%s", PJLSetPaperSize, paper_size_list[opt->paper].pjl);
	strcat (pjl, str);
	sprintf (str, "%s%s", PJLSetCollate, collate_list[opt->collate].pjl);
	strcat (pjl, str);
	if (opt->collate == COLLATE_OFF) {
		sprintf (str, "%s%d\x0a", PJLSetCopies, opt->copies);
	} else {
		sprintf (str, "%s%d\x0a", PJLSetQty, opt->copies);
	}
	strcat (pjl, str);
	strcat (pjl, PJLSetJobAttribute);
	strcat (pjl, PJLAttributeFstp);
	strcat (pjl, PJLSetJobAttribute);
	strcat (pjl, PJLAttributeLstp);
	strcat (pjl, PJLSetJobAttribute);
	sprintf (str, "%s%s", PJLAttributeLout, layout_list[opt->layout].pjl);
	strcat (pjl, str);
	if (opt->layout == LOUT_LEFTUP || opt->layout == LOUT_CENTER) {
		strcat (pjl, PJLSetJobAttribute);
		strcat (pjl, PJLAttributeZoom);
	}
	sprintf (str, "%s%s", PJLSetDuplex, duplex_list[opt->duplex].pjl);
	strcat (pjl, str);
	if (opt->duplex != DUPLEX_OFF) {
		sprintf (str, "%s%s", PJLSetBinding, binding_list[opt->duplex].pjl);
		strcat (pjl, str);
	}
	if (opt->job_type != JOB_NORMAL) {
		sprintf (str, "%s%s", PJLSetHold, job_type_list[opt->job_type].pjl);
		strcat (pjl, str);
		if (opt->job_type == JOB_SECURITY) {
			strcat (pjl, PJLSetHoldType);
			if (opt->hold_key[0] != 0x00) {
				if (Decode (opt->hold_key, decode_buff, &decode_len)) {
   		 			decode_buff[decode_len] = 0;
				} else {
					strcpy (decode_buff, opt->hold_key);
				}
				sprintf (str, "%s\"%s\"\x0a", PJLSetHoldKey, decode_buff);
				strcat (pjl, str);
			}
		}
		sprintf (str, "%s\"%s\"\x0a", PJLSetUserName, opt->user_name);
		strcat (pjl, str);
		sprintf (str, "%s\"%s\"\x0a", PJLSetJobName, opt->job_name);
		strcat (pjl, str);
	}
	strcat (pjl, PJLSetJobAttribute);
	strcat (pjl, PJLAttributeAnnt);
	if (opt->input_slot == TRAY_AUTO) {
		strcat (pjl, PJLSetAutoSelectOn);
	} else {
		strcat (pjl, PJLSetAutoSelectOff);
		sprintf (str, "%s%s", PJLSetMediaSource, input_slot_list[opt->input_slot].pjl);
		strcat (pjl, str);
	}
	if (opt->media_type == MEDIA_NORMAL) {
		sprintf (str, "%s%s", PJLSetMediaType, media_type_list[opt->media_type].pjl);
		strcat (pjl, str);
	}
	if (opt->staple != STAPLE_OFF || opt->punch != PUNCH_OFF) {
		strcat (pjl, PJLSetOutBin100);
		strcat (pjl, PJLSetFinishOn);
		if (opt->staple != STAPLE_OFF) {
			sprintf (str, "%s%s", PJLSetStaple, staple_list[opt->staple].pjl);	
			strcat (pjl, str);
		}
		if (opt->punch != PUNCH_OFF) {
			strcat (pjl, PJLSetPunch);
			strcat (pjl, PJLSetJobAttribute);
			strcat (pjl, PJLAttributePunch);	
		}			
	} else {
		strcat (pjl, PJLSetOutMain);
	}
//	if (opt->banner != BANNER_OFF) {
//		strcat (pjl, PJLSetJobAttribute);
//		strcat (pjl, PJLAttributeBanr);	
//		strcat (pjl, PJLSetJobAttribute);
//		sprintf (ss, "%s%s\"\x0a", PJLAttributeJoau, opt->bn_user);	
//		strcat (pjl, ss);
//	}
	strcat (pjl, PJLSetJobAttribute);
	sprintf (str, "%s%u\"\x0a", PJLAttributeFsize, opt->size);
	strcat (pjl, str);
	strcat (pjl, PJLSetTimeOut);
	strcat (pjl, PJLEnterLanguage);
	return (0);
}

//==============================================
// fxExecFilter
//==============================================
int fxExecFilter (char* arg[])
{
	int pid;
	int status;
	int	rtn = 0;

	sigignore (SIGCHLD);

	switch (pid=fork()) {
	case -1:	// error
		fputs ("ERROR: ps2pdf proccess is not exist.", stderr);
		rtn = -2;
	case 0:		// child proccess
		sleep (5);
		execvp (arg[0], arg);
		fputs ("ERROR: ps2pdf proccess is abnormal end.", stderr);
		rtn = -1;
		exit (-1);
		break;
	default:	// owner process
		wait (&status);
	}

	return (rtn);
}
