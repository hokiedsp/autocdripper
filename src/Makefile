CC = g++
CFLAGS = -Wall -std=c++11 -I/usr/include/glib-2.0 \
         -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/libxml2 \
         -I/usr/include/x86_64-linux-gnu

MAIN = autocdripper
SRCS = CSourceCdda.cpp CSinkBase.cpp CSinkWav.cpp\
       CSinkWavPack.cpp CTagsGeneric.cpp CTagsAPEv2.cpp SCueSheet.cpp CDbFreeDb.cpp\
       CDbMusicBrainz.cpp enums.cpp CDbDiscogs.cpp CUtilJson.cpp\
       CDbDiscogsElem.cpp CUtilUrl.cpp utils.cpp CDbLastFm.cpp CDbLastFmElem.cpp\
       CUtilXmlTree.cpp CUtilXml.cpp CDbMusicBrainzElem.cpp\
       CDbMusicBrainzElemCAA.cpp CDbAmazon.cpp CDbAmazonElem.cpp\
       CCdRipper.cpp CCueSheetBuilder.cpp autocdripper.cpp\
       CFileNameGenerator.cpp
LIBS = -lwavpack -lcdio -lcdio_cdda -lcdio_paranoia -lcddb -lcurl -ljansson -lxml2\
       -L/usr/lib/x86_64-linux-gnu -lboost_regex -licuuc -licudata
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
#SRCS = discogsdemo.cpp SCueSheet.cpp CDbDiscogs.cpp CUtilUrl.cpp CDbJsonBase.cpp
#MAIN = lastfmdemo
#SRCS = lastfmdemo.cpp SCueSheet.cpp CDbLastFm.cpp CUtilUrl.cpp CDbJsonBase.cpp
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

