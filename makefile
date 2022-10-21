FILES = src/reciver/dns_reciver_events.c src/reciver/dns_reciver.c src/sender/dns_sender_events.c src/sender/dns_senderc
CFLAGS = -Wall -lpcap 
CC = gcc
OUTPUT = sender reciver

.PHONY: all sender reciver clean
all: sender reciver

sender:

reciver:

clean: $(RM) $(OUTPUT)