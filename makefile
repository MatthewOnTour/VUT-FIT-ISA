CFLAGS=-Wall -pipe -pedantic -Werror -g
FILES_SENDER=sender/dns_sender.c base32.c dns.c sender/dns_sender_events.c
FILES_RECEIVER=receiver/dns_receiver.c base32.c dns.c receiver/dns_receiver_events.c


all : $sender $receiver

sender : $sender
receiver : $receiver


$sender : $(FILES_SENDER)
	gcc $(FILES_SENDER) $(CFLAGS) -o dns_sender


$receiver : $(FILES_RECEIVER)
		gcc $(FILES_RECEIVER) $(CFLAGS) -o dns_receiver 

clean :
	rm -f *.o dns_sender dns_receiver