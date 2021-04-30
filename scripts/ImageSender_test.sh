echo "**************************************Running the test for ImageSender coupled model************************"
echo "Using input from file:"
echo " ../input_data/ImageSender_input_test_ack_in.txt"
echo " ../input_data/ImageSender_input_test_start_signal.txt"
../bin/IMAGESENDER_TEST
echo "**************************************Finished the testing for ImageSender coupled model*******************"
echo "Check output messages in file:"
echo "../simulation_results/ImageSender_test_output_messages.txt"
