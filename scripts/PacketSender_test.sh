echo "**************************************Running the test for PacketSender coupled model************************"
echo "Using input from file:"
echo " ../input_data/packetSender_input_test_ack_in.txt"
echo " ../input_data/packetSender_input_test_packet_done.txt"
../bin/PACKETSENDER_TEST
echo "**************************************Finished the testing for PacketSender coupled model****************8****"
echo "Check output messages in file:"
echo "../simulation_results/packetSender_test_output_messages.txt"
