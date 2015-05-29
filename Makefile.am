cupsfilterdir = /usr/lib/cups/filter
cupsppddir = /usr/share/cups/model/FujiXerox/en
cupsmimedir = /etc/cups

cupsfilter_PROGRAMS = pstopdffx pdftopjlfx pdftopdffx
EXTRA_DIST = fxlinuxprint.ppd mimefx.types mimefx.convs
cupsppd_DATA = fxlinuxprint.ppd
cupsmime_DATA = mimefx.types mimefx.convs

pstopdffx_SOURCES = fxlinuxprint.c fxlinuxprint.h codec.c codec.h
pstopdffx_LDFLAGS = -lcups
pstopdffx_CFLAGS = -D__PS2PDF__

pdftopjlfx_SOURCES = fxlinuxprint.c fxlinuxprint.h codec.c codec.h
pdftopjlfx_LDFLAGS = -lcups

pdftopdffx_SOURCES = pdftopdffx.c
