//Cadmium Simulator headers
#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/dynamic_model.hpp>
#include <cadmium/modeling/dynamic_model_translator.hpp>
#include <cadmium/engine/pdevs_dynamic_runner.hpp>
#include <cadmium/logger/common_loggers.hpp>

//Time class header
#include <NDTime.hpp>

//Messages structures
#include "../data_structures/packet_format.hpp"

//Atomic model headers
#include <cadmium/basic_model/pdevs/iestream.hpp> //Atomic model for inputs
#include "../atomics/PacketForwarder.hpp"

//C++ libraries
#include <iostream>
#include <string>

using namespace std;
using namespace cadmium;
using namespace cadmium::basic_models::pdevs;

using TIME = NDTime;

/***** Define input port for coupled models *****/
struct inp_packetIn : public cadmium::in_port<Packet_t>{};
struct inp_resendIn : public cadmium::in_port<Packet_t> {};

/***** Define output ports for coupled model *****/
struct outp_nextPacket : public cadmium::out_port<int>{};
struct outp_packetOut : public cadmium::out_port<Packet_t>{};

///***** Define output ports for coupled model *****/
//struct outp : public cadmium::out_port<Packet_t>{};

/****** Input Reader atomic model declaration *******************/
template<typename T>
class InputReader_Packet_t : public iestream_input<Packet_t,T> {
    public:
        InputReader_Packet_t () = default;
        InputReader_Packet_t (const char* file_path) : iestream_input<Packet_t,T>(file_path) {}
};


int main(){

    /****** Input Reader atomic model instantiation *******************/
    const char * i_input_data_packetIn = "../input_data/packetForwarder_input_test_packetIn.txt";
    shared_ptr<dynamic::modeling::model> input_reader_packetIn = dynamic::translate::make_dynamic_atomic_model<InputReader_Packet_t, TIME, const char* >("input_reader_packetIn" , std::move(i_input_data_packetIn));

    const char * i_input_data_resendIn = "../input_data/packetForwarder_input_test_resendIn.txt";
    shared_ptr<dynamic::modeling::model> input_reader_resendIn = dynamic::translate::make_dynamic_atomic_model<InputReader_Packet_t, TIME, const char* >("input_reader_resendIn" , std::move(i_input_data_resendIn));


    /****** Reciever atomic model instantiation *******************/
    shared_ptr<dynamic::modeling::model> PacketForwarder1 = dynamic::translate::make_dynamic_atomic_model<PacketForwarder, TIME>("PacketForwarder1");

    /*******TOP MODEL********/
    dynamic::modeling::Ports iports_TOP = {};
    dynamic::modeling::Ports oports_TOP = {typeid(outp_packetOut),typeid(outp_nextPacket)};
    dynamic::modeling::Models submodels_TOP = {input_reader_packetIn, input_reader_resendIn, PacketForwarder1};
    dynamic::modeling::EICs eics_TOP = {};
    dynamic::modeling::EOCs eocs_TOP = {
        dynamic::translate::make_EOC<PacketForwarder_defs::packetOut,outp_packetOut>("PacketForwarder1"),
        dynamic::translate::make_EOC<PacketForwarder_defs::nextPacket,outp_nextPacket>("PacketForwarder1")
    };
    dynamic::modeling::ICs ics_TOP = {
        dynamic::translate::make_IC<iestream_input_defs<Packet_t>::out,PacketForwarder_defs::packetIn>("input_reader_packetIn","PacketForwarder1"),
        dynamic::translate::make_IC<iestream_input_defs<Packet_t>::out,PacketForwarder_defs::resendIn>("input_reader_resendIn","PacketForwarder1"),

    };
    shared_ptr<dynamic::modeling::coupled<TIME>> TOP;
    TOP = make_shared<dynamic::modeling::coupled<TIME>>(
        "TOP", submodels_TOP, iports_TOP, oports_TOP, eics_TOP, eocs_TOP, ics_TOP
    );

    /*************** Loggers *******************/
    static ofstream out_messages("../simulation_results/packetForwarder_test_output_messages.txt");
    struct oss_sink_messages{
        static ostream& sink(){
            return out_messages;
        }
    };
    static ofstream out_state("../simulation_results/packetForwarder_test_output_state.txt");
    struct oss_sink_state{
        static ostream& sink(){
            return out_state;
        }
    };

    using state=logger::logger<logger::logger_state, dynamic::logger::formatter<TIME>, oss_sink_state>;
    using log_messages=logger::logger<logger::logger_messages, dynamic::logger::formatter<TIME>, oss_sink_messages>;
    using global_time_mes=logger::logger<logger::logger_global_time, dynamic::logger::formatter<TIME>, oss_sink_messages>;
    using global_time_sta=logger::logger<logger::logger_global_time, dynamic::logger::formatter<TIME>, oss_sink_state>;

    using logger_top=logger::multilogger<state, log_messages, global_time_mes, global_time_sta>;

    /************** Runner call ************************/
    dynamic::engine::runner<NDTime, logger_top> r(TOP, {0});
    r.run_until(NDTime("02:00:00:000"));
    return 0;
}
