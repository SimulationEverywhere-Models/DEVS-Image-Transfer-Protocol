CC=g++
CFLAGS=-std=c++17

INCLUDECADMIUM=-I ../../cadmium/include
INCLUDEDESTIMES=-I ../../DESTimes/include

#CREATE BIN AND BUILD FOLDERS TO SAVE THE COMPILED FILES DURING RUNTIME
bin_folder := $(shell mkdir -p bin)
build_folder := $(shell mkdir -p build)
results_folder := $(shell mkdir -p simulation_results)

#TARGET TO COMPILE ALL THE TESTS TOGETHER (NOT SIMULATOR)
packet_format.o: data_structures/packet_format.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) data_structures/packet_format.cpp -o build/packet_format.o

main_ITP_top_model.o: top_model/main_ITP_top_model.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) top_model/main_ITP_top_model.cpp -o build/main_ITP_top_model.o
#
# main_subnet_test.o: test/main_subnet_test.cpp
# 	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) test/main_subnet_test.cpp -o build/main_subnet_test.o
#
main_imageSender_test.o: test/main_imageSender_test.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) test/main_imageSender_test.cpp -o build/main_imageSender_test.o

main_packetSender_test.o: test/main_packetSender_test.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) test/main_packetSender_test.cpp -o build/main_packetSender_test.o

main_receiver_test.o: test/main_receiver_test.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) test/main_receiver_test.cpp -o build/main_receiver_test.o

main_networkDelay_test.o: test/main_networkDelay_test.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) test/main_networkDelay_test.cpp -o build/main_networkDelay_test.o

main_switchBuffer_test.o: test/main_switchBuffer_test.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) test/main_switchBuffer_test.cpp -o build/main_switchBuffer_test.o

main_packetGenerator_test.o: test/main_packetGenerator_test.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) test/main_packetGenerator_test.cpp -o build/main_packetGenerator_test.o

main_retransmission_test.o: test/main_retransmission_test.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) test/main_retransmission_test.cpp -o build/main_retransmission_test.o

main_packetForwarder_test.o: test/main_packetForwarder_test.cpp
	$(CC) -g -c $(CFLAGS) $(INCLUDECADMIUM) $(INCLUDEDESTIMES) test/main_packetForwarder_test.cpp -o build/main_packetForwarder_test.o
# tests: main_subnet_test.o main_imageSender_test.o main_receiver_test.o message.o
# 		$(CC) -g -o bin/SUBNET_TEST build/main_subnet_test.o build/message.o
# 		$(CC) -g -o bin/IMAGESENDER_TEST build/main_imageSender_test.o build/message.o
# 		$(CC) -g -o bin/RECEIVER_TEST build/main_receiver_test.o build/message.o

tests: main_receiver_test.o packet_format.o main_packetGenerator_test.o main_retransmission_test.o main_packetForwarder_test.o main_packetSender_test.o main_imageSender_test.o main_switchBuffer_test.o main_networkDelay_test.o
		$(CC) -g -o bin/IMAGESENDER_TEST build/main_imageSender_test.o build/packet_format.o
		$(CC) -g -o bin/PACKETSENDER_TEST build/main_packetSender_test.o build/packet_format.o
		$(CC) -g -o bin/RECEIVER_TEST build/main_receiver_test.o build/packet_format.o
		$(CC) -g -o bin/PACKETGENERATOR_TEST build/main_packetGenerator_test.o build/packet_format.o
		$(CC) -g -o bin/RETRANSMISSION_TEST build/main_retransmission_test.o build/packet_format.o
		$(CC) -g -o bin/PACKETFORWARDER_TEST build/main_packetForwarder_test.o build/packet_format.o
		$(CC) -g -o bin/SWITCHBUFFER_TEST build/main_switchBuffer_test.o build/packet_format.o
		$(CC) -g -o bin/NETWORKDELAY_TEST build/main_networkDelay_test.o build/packet_format.o


#TARGET TO COMPILE ONLY TOP module ITP SIMULATOR
simulator: main_ITP_top_model.o packet_format.o
	$(CC) -g -o bin/ITP_TOP_MODEL_TEST build/main_ITP_top_model.o build/packet_format.o

#TARGET TO COMPILE EVERYTHING (ITP SIMULATOR + TESTS TOGETHER)
all: simulator tests


#CLEAN COMMANDS
clean:
	rm -f bin/* build/*
