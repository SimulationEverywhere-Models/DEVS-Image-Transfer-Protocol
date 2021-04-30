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
#include "../atomics/PacketGenerator.hpp"
#include "../atomics/Retransmission.hpp"

//C++ libraries
#include <iostream>
#include <string>

using namespace std;
using namespace cadmium;
using namespace cadmium::basic_models::pdevs;

using TIME = NDTime;

/***** Define input port for coupled models *****/
struct inp_s_start : public cadmium::in_port<int>{};
struct inp_s_ack : public cadmium::in_port<Packet_t>{};
struct inp_ps_packet_done : public cadmium::in_port<Packet_t>{};
struct inp_ps_ack : public cadmium::in_port<Packet_t>{};

/***** Define output ports for coupled model *****/
struct outp_s_packet_out : public cadmium::out_port<Packet_t>{};
struct outp_ps_next_packet : public cadmium::out_port<int>{};
struct outp_ps_size_change : public cadmium::out_port<int>{};
struct outp_ps_packet_out : public cadmium::out_port<Packet_t>{};
struct outp_packet_out : public cadmium::out_port<Packet_t>{};

/****** Input Reader atomic model declaration *******************/
template<typename T>
class InputReader_Packet_t : public iestream_input<Packet_t,T> {
    public:
        InputReader_Packet_t () = default;
        InputReader_Packet_t (const char* file_path) : iestream_input<Packet_t,T>(file_path) {}
};
template<typename T>
class InputReader_Int : public iestream_input<int,T> {
    public:
        InputReader_Int () = default;
        InputReader_Int (const char* file_path) : iestream_input<int,T>(file_path) {}
};

int main(){


    /****** Input Reader atomic models instantiation *******************/
    const char * i_input_start_signal = "../input_data/ImageSender_input_test_start_signal.txt";
    shared_ptr<dynamic::modeling::model> input_reader_start_signal = dynamic::translate::make_dynamic_atomic_model<InputReader_Int, TIME, const char* >("input_reader_start_signal" , move(i_input_start_signal));

    const char * i_input_data_ack = "../input_data/ImageSender_input_test_ack_in.txt";
    shared_ptr<dynamic::modeling::model> input_reader_ack = dynamic::translate::make_dynamic_atomic_model<InputReader_Packet_t , TIME, const char* >("input_reader_ack" , move(i_input_data_ack));


    /****** Sender atomic model instantiation *******************/
    shared_ptr<dynamic::modeling::model> PacketForwarder1 = dynamic::translate::make_dynamic_atomic_model<PacketForwarder, TIME>("PacketForwarder1");
    shared_ptr<dynamic::modeling::model> PacketGenerator1 = dynamic::translate::make_dynamic_atomic_model<PacketGenerator, TIME>("PacketGenerator1");
    shared_ptr<dynamic::modeling::model> Retransmission1 = dynamic::translate::make_dynamic_atomic_model<Retransmission, TIME>("Retransmission1");

    /*******PACKET SENDER COUPLED MODEL********/
    dynamic::modeling::Ports iports_Packet_Sender = {typeid(inp_ps_packet_done),typeid(inp_ps_ack)};
    dynamic::modeling::Ports oports_Packet_Sender = {typeid(outp_ps_next_packet),typeid(outp_ps_size_change),typeid(outp_ps_packet_out)};
    dynamic::modeling::Models submodels_Packet_Sender = {PacketForwarder1, Retransmission1};
    dynamic::modeling::EICs eics_Packet_Sender = {
        dynamic::translate::make_EIC<inp_ps_packet_done, PacketForwarder_defs::packetIn>("PacketForwarder1"),
        dynamic::translate::make_EIC<inp_ps_ack, Retransmission_defs::ackIn>("Retransmission1")
    };
    dynamic::modeling::EOCs eocs_Packet_Sender = {
        dynamic::translate::make_EOC<PacketForwarder_defs::packetOut,outp_ps_packet_out>("PacketForwarder1"),
        dynamic::translate::make_EOC<PacketForwarder_defs::nextPacket,outp_ps_next_packet>("PacketForwarder1"),
        dynamic::translate::make_EOC<Retransmission_defs::sizeChange,outp_ps_size_change>("Retransmission1")
    };
    dynamic::modeling::ICs ics_Packet_Sender = {
        dynamic::translate::make_IC<Retransmission_defs::resend,PacketForwarder_defs::resendIn>("Retransmission1","PacketForwarder1"),
        dynamic::translate::make_IC<PacketForwarder_defs::packetOut,Retransmission_defs::packetIn>("PacketForwarder1","Retransmission1")
    };
    shared_ptr<dynamic::modeling::coupled<TIME>> PACKET_SENDER;
    PACKET_SENDER = make_shared<dynamic::modeling::coupled<TIME>>(
        "Packet_Sender", submodels_Packet_Sender, iports_Packet_Sender, oports_Packet_Sender, eics_Packet_Sender, eocs_Packet_Sender, ics_Packet_Sender
    );

    /*******SENDER COUPLED MODEL********/
    dynamic::modeling::Ports iports_Sender = {typeid(inp_s_start),typeid(inp_s_ack)};
    dynamic::modeling::Ports oports_Sender = {typeid(outp_s_packet_out)};
    dynamic::modeling::Models submodels_Sender = {PacketGenerator1, PACKET_SENDER};
    dynamic::modeling::EICs eics_Sender = {
        dynamic::translate::make_EIC<inp_s_start, PacketGenerator_defs::startSignal>("PacketGenerator1"),
        dynamic::translate::make_EIC<inp_s_ack, inp_ps_ack>("Packet_Sender")
    };
    dynamic::modeling::EOCs eocs_Sender = {
        dynamic::translate::make_EOC<outp_ps_packet_out,outp_s_packet_out>("Packet_Sender"),
    };
    dynamic::modeling::ICs ics_Sender = {
        dynamic::translate::make_IC<PacketGenerator_defs::packetDone,inp_ps_packet_done>("PacketGenerator1","Packet_Sender"),
        dynamic::translate::make_IC<outp_ps_next_packet,PacketGenerator_defs::nextPacket>("Packet_Sender","PacketGenerator1"),
        dynamic::translate::make_IC<outp_ps_size_change,PacketGenerator_defs::sizeChange>("Packet_Sender","PacketGenerator1"),
    };
    shared_ptr<dynamic::modeling::coupled<TIME>> SENDER;
    SENDER = make_shared<dynamic::modeling::coupled<TIME>>(
        "Sender", submodels_Sender, iports_Sender, oports_Sender, eics_Sender, eocs_Sender, ics_Sender
    );

    /*******TOP MODEL********/
    dynamic::modeling::Ports iports_TOP = {};
    dynamic::modeling::Ports oports_TOP = {typeid(outp_packet_out)};
    dynamic::modeling::Models submodels_TOP = {input_reader_start_signal, input_reader_ack, SENDER};
    dynamic::modeling::EICs eics_TOP = {};
    dynamic::modeling::EOCs eocs_TOP = {
        dynamic::translate::make_EOC<outp_s_packet_out,outp_packet_out>("Sender")
    };
    dynamic::modeling::ICs ics_TOP = {
        dynamic::translate::make_IC<iestream_input_defs<int>::out,inp_s_start>("input_reader_start_signal","Sender"),
        dynamic::translate::make_IC<iestream_input_defs<Packet_t>::out,inp_s_ack>("input_reader_ack","Sender")
    };
    shared_ptr<dynamic::modeling::coupled<TIME>> TOP;
    TOP = make_shared<dynamic::modeling::coupled<TIME>>(
        "TOP", submodels_TOP, iports_TOP, oports_TOP, eics_TOP, eocs_TOP, ics_TOP
    );

    /*************** Loggers *******************/
    static ofstream out_messages("../simulation_results/ImageSender_test_output_messages.txt");
    struct oss_sink_messages{
        static ostream& sink(){
            return out_messages;
        }
    };
    static ofstream out_state("../simulation_results/ImageSender_test_output_state.txt");
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
    r.run_until(NDTime("04:00:00:000"));
    return 0;
}
