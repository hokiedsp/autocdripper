CC = g++
CFLAGS = -Wall -std=c++11

#MAIN = cdripdemo
#SRCS = cdripdemo.cpp CSourceCdda.cpp CSinkBase.cpp CSinkWav.cpp CSinkWavPack.cpp CTagsGeneric.cpp CTagsAPEv2.cpp
#LIBS = -lwavpack -lcdio -lcdio_cdda -lcdio_paranoia
#LDFLAGS = -Wall

#MAIN = cdinfodemo
#SRCS = cdinfodemo.cpp CSourceCdda.cpp SCueSheet.cpp CDbCDDB.cpp
#LIBS = -lcdio -lcdio_cdda -lcdio_paranoia -lcddb
#LDFLAGS =

MAIN = cdinfodemo
SRCS = cdinfodemo.cpp CSourceCdda.cpp SCueSheet.cpp CDbMusicBrainz.cpp
LIBS = -lcdio -lcdio_cdda -lcdio_paranoia -lmusicbrainz5 -ldiscid -lcoverart
LDFLAGS =

OBJS = $(SRCS:.cpp=.o)

.PHONY: clean

$(MAIN): $(OBJS) 
	$(CC) $(CFLAGS) -o $(MAIN) $(OBJS) $(LDFLAGS) $(LIBS)
	$(RM) *.o

.cpp.o:
	$(CC) $(CFLAGS) -c $<  -o $@

clean:
	$(RM) *.o *~ $(MAIN)

