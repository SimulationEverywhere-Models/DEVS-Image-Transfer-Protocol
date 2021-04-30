This folder contains the Image Transfer Protocol DEVS model implemented in Cadmium

/**************************/
/****FILES ORGANIZATION****/
/**************************/

README.txt	
ImageTransferProtocol.doc
makefile

atomics [This folder contains atomic models implemented in Cadmium]
	NetworkDelay.hpp
	PacketForwarder.hpp
	PacketGenerator.hpp
	Receiver.hpp
	Retransmission.hpp
	SwitchBuffer.hpp
bin [This folder will be created automatically the first time you compile the poject.
     It will contain all the executables]
build [This folder will be created automatically the first time you compile the poject.
       It will contain all the build files (.o) generated during compilation]
data_structures [This folder contains message data structure used in the model]
	packet_format.cpp
	packet_format.hpp
input_data [This folder contains all the input data to run the model and the tests]
    ImageSender_input_test_ack_in.txt
    ImageSender_input_test_start_signal.txt
    ITP_input_test_start_signal.txt
    NetworkDelay_input_test.txt
    packetForwarder_input_test_packetIn.txt
    packetForwarder_input_test_resendIn.txt
    packetGenerator_input_test_nextPacket.txt
    packetGenerator_input_test_sizeChange.txt
    packetGenerator_input_test_startSignal.txt
    packetSender_input_test_ack_in.txt
    packetSender_input_test_packet_done.txt
    receiver_input_test.txt
    Retransmission_input_test_ackIn.txt
    Retransmission_input_test_packetIn.txt
    SwitchBuffer_input_test.txt
simulation_results [This folder will be created automatically the first time you compile the poject.
                    It will store the outputs from your simulations and tests]
test [This folder contains the testing for the atomic and coupled models(excluding top model)]
    main_imageSender_test.cpp
    main_networkDelay_test.cpp
    main_packetForwarder_test.cpp
    main_packetGenerator_test.cpp
    main_packetSender_test.cpp
    main_receiver_test.cpp
    main_retransmission_test.cpp
    main_switchBuffer_test.cpp
top_model [This folder contains the Image Transfer Protocol top model]	
	main_ITP_top_model.cpp
	
/*************/
/****STEPS****/
/*************/

0 - ImageTransferProtocol.doc contains the explanation and testing of this model

1 - Update include path in the makefile in this folder and subfolders. You need to update the following lines:
	INCLUDECADMIUM=-I ../../cadmium/include
	INCLUDEDESTIMES=-I ../../DESTimes/include
    Update the relative path to cadmium/include from the folder where the makefile is. You need to take into account where you copied the folder during the installation process
	Example: INCLUDECADMIUM=-I ../../cadmium/include
	Do the same for the DESTimes library
    NOTE: if you follow the step by step installation guide you will not need to update these paths.
2 - Compile the project and the tests
	1 - Open the terminal (Ubuntu terminal for Linux and Cygwin for Windows) in the ITP folder
	3 - To compile the project and the tests, type in the terminal:
			make clean; make all
3 - ******************************************************Run the testing for atomic/coupled/top model******************************************************
    Step 1: cd scripts/
            You should see all the .sh file below:
            
            ImageSender_test.sh    NetworkDelay_test.sh     PacketGenerator_test.sh  Receiver_test.sh        SwitchBuffer_test.sh
            ITP_top_model_test.sh  PacketForwarder_test.sh  PacketSender_test.sh     Retransmission_test.sh

            Make sure they are excutable. If they are not, issue chmod a+rx ./*
            
    Step 2: ./PacketGenerator_test.sh
    
            You should see the following terminal output.
            It specifies what input files are used and where the output file is created.
            You can copy/past the file path and open the file.
            For example (if you are on Linux): gedit ../simulation_results/packetGenerator_test_output_messages.txt
            
            **************************************Running the test for PacketGenerator atomic model************************
            Using input from file:
             ../input_data/packetGenerator_input_test_nextPacket.txt
             ../input_data/packetGenerator_input_test_sizeChange.txt
             ../input_data/packetGenerator_input_test_startSignal.txt
            **************************************Finished the testing for PacketGenerator atomic model****************8****
            Check output messages in file:
            ../simulation_results/packetGenerator_test_output_messages.txt
            
    Step 3: Run all the rest testings with different models following step 2
    
    Note: ITP_top_model_test.sh is the top model
          ImageSender_test.sh, PacketSender_test.sh are coupled models
