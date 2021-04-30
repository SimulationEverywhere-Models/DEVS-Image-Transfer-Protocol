echo "**************************************Running the test for Retransmission atomic model************************"
echo "Using input from file:"
echo " ../input_data/Retransmission_input_test_ackIn.txt"
echo " ../input_data/Retransmission_input_test_packetIn.txt"
../bin/RETRANSMISSION_TEST
echo "**************************************Finished the testing for Retransmission atomic model****************8****"
echo "Check output messages in file:"
echo "../simulation_results/Retransmission_test_output_messages.txt"
