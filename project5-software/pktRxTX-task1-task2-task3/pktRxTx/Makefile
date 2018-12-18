###
 # (C) Copyright 2018 ICT.
 # Wenqing Wu <wuwenqing@ict.ac.cn>
 #
 # This program is free software; you can redistribute it and/or modify
 # it under the terms of the GNU General Public License as published by
 # the Free Software Foundation; either version 2 of the License, or
 # (at your option) any later version.
 #
 # This program is distributed in the hope that it will be useful,
 # but WITHOUT ANY WARRANTY; without even the implied warranty of
 # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 # GNU General Public License for more details.
 #
 ##

CC := gcc

ifeq ($(OS),Windows_NT)
	PCAP_PATH := ./pcap/lib
	CFLAGS := -g -O -I ./pcap/include
	LDFLAGS := -L$(PCAP_PATH) -lwpcap
else
	LDLIBS	:= -lpcap -lpthread -lnet
	
	LDFLAGS :=
#	PREFIX := x86
	
	CFLAGS := -g -O
	LDFLAGS += $(LDLIBS)
endif

PRG	= pktRxTx
OBJ	= pktRxTx.o print.o
SRC	= $(OBJ:.o=.c)
DEPEND	= $(OBJ:.o=.d)

.PHONY: all
all: $(PRG)

%pktRxTx: $(SRC)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f *.o $(PRG)

