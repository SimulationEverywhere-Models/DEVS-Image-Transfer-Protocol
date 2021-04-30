echo "**************************************Running the test for PacketForwarder atomic model************************"
echo "Using input from file:"
echo " ../input_data/packetForwarder_input_test_packetIn.txt"
echo " ../input_data/packetForwarder_input_test_resendIn.txt"
../bin/PACKETFORWARDER_TEST
echo "**************************************Finished the testing for PacketForwarder atomic model****************8****"
echo "Check output messages in file:"
echo "../simulation_results/packetForwarder_test_output_messages.txt"
