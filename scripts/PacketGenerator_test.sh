echo "**************************************Running the test for PacketGenerator atomic model************************"
echo "Using input from file:"
echo " ../input_data/packetGenerator_input_test_nextPacket.txt"
echo " ../input_data/packetGenerator_input_test_sizeChange.txt"
echo " ../input_data/packetGenerator_input_test_startSignal.txt"
../bin/PACKETGENERATOR_TEST
echo "**************************************Finished the testing for PacketGenerator atomic model****************8****"
echo "Check output messages in file:"
echo "../simulation_results/packetGenerator_test_output_messages.txt"
