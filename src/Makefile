CC = g++
CFLAGS = -Wall -std=c++11 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include

MAIN = autocdripper
SRCS = autocdripper.cpp CCdRipper.cpp CSourceCdda.cpp CSinkBase.cpp CSinkWav.cpp\
       CSinkWavPack.cpp CTagsGeneric.cpp CTagsAPEv2.cpp SCueSheet.cpp CDbFreeDb.cpp\
       CDbMusicBrainz.cpp CCueSheetBuilder.cpp enums.cpp CDbDiscogs.cpp CDbElemJsonBase.cpp\
       CDbDiscogsElem.cpp CDbHttpBase.cpp
LIBS = -lwavpack -lcdio -lcdio_cdda -lcdio_paranoia -lcddb -lmusicbrainz5 -ldiscid -lcoverart\
       -lcurl -ljansson
LDFLAGS = -Wall -pthread

#MAIN = cdinfodemo
#SRCS = cdinfodemo.cpp CSourceCdda.cpp SCueSheet.cpp CDbCDDB.cpp
#LIBS = -lcdio -lcdio_cdda -lcdio_paranoia -lcddb
#LDFLAGS =

#MAIN = cdinfodemo
#SRCS = cdinfodemo.cpp CSourceCdda.cpp SCueSheet.cpp CDbMusicBrainz.cpp
#LIBS = -lcdio -lcdio_cdda -lcdio_paranoia -lmusicbrainz5 -ldiscid -lcoverart
#LDFLAGS =

#MAIN = discogsdemo
#SRCS = discogsdemo.cpp SCueSheet.cpp CDbDiscogs.cpp CDbHttpBase.cpp CDbJsonBase.cpp
#MAIN = lastfmdemo
#SRCS = lastfmdemo.cpp SCueSheet.cpp CDbLastFm.cpp CDbHttpBase.cpp CDbJsonBase.cpp
#LIBS = -lcurl -ljansson
#LDFLAGS =

#MAIN = gkeyfiletest
#SRCS = gkeyfiletest.cpp CGKeyFileAutoCDRipper.cpp CGKeyFileBase.cpp enums.cpp
#LIBS = -lglib-2.0
#LDFLAGS =

OBJS = $(SRCS:.cpp=.o)

.PHONY: clean

$(MAIN): $(OBJS) 
	$(CC) $(CFLAGS) -o $(MAIN) $(OBJS) $(LDFLAGS) $(LIBS)
	$(RM) *.o

.cpp.o:
	$(CC) $(CFLAGS) -c $<  -o $@

clean:
	$(RM) *.o *~ $(MAIN)

