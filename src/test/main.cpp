#include <catch2/catch_all.hpp>
#include <fmt/format.h>
using fmt::print;

#include <G4Exception.hh>

#include <core/utils/simulator.h>


using namespace allpix;


/// Taken from examples/magnetic_field/magnetic_field.conf
TEST_CASE("MagneticField")
{
    Log::addStream(std::cout);

    ConfigManagerSettings settings;

    settings.globalcfg = CreateConfiguration("Allpix", {
        {"number_of_events", "25"},
        {"log_level", "DEBUG"},
        {"log_format", "LONG"},
    });

    settings.modules = {
        CreateConfiguration("GeometryBuilderGeant4", {}),
        CreateConfiguration("MagneticFieldReader", {
            {"model", "constant"},
            {"magnetic_field", "0mT 3.8T 0T"}
        }),
        CreateConfiguration("DepositionGeant4", {
            {"physics_list", "FTFP_BERT_LIV"},
            {"particle_type", "e-"},
            {"source_energy", "0.1GeV"},
            {"source_position", "33um 26um -500um"},
            {"source_type", "beam"},
            {"beam_size", "2mm"},
            {"beam_direction", "0 0 1"},
            {"number_of_particles", "1"},
            {"max_step_length", "1um"},
        }),
        CreateConfiguration("ElectricFieldReader", {
            {"model", "linear"},
            {"voltage", "-150V"}
        }),
        CreateConfiguration("GenericPropagation", {
            {"temperature", "293K"},
            {"charge_per_step", "10"},
            {"propagate_holes", "1"},
            {"timestep_min", "0.1ns"},
            {"timestep_max", "0.5ns"},
            {"output_plots", "1"},
        }),
        CreateConfiguration("SimpleTransfer", {
            {"max_depth_distance", "5um"},
        }),
        CreateConfiguration("DefaultDigitizer", {}),
        CreateConfiguration("DetectorHistogrammer", {
            {"name", "detector1"},
        }),
        CreateConfiguration("DetectorHistogrammer", {
            {"name", "detector2"},
        }),
    };

    settings.detector_configs = {
        CreateConfiguration("detector1", {
            {"type", "cmsp1"},
            {"position", "0 0 0"},
            {"orientation", "0 0 0"},
        }),
        CreateConfiguration("detector2", {
            {"type", "cmsp1"},
            {"position", "10um 80um 10mm"},
            {"orientation", "0 19deg 0"},
        }),
    };

    AllPixSimulator simulator(settings);

    try
    {
        simulator.run();
    }
    catch(Exception& e)
    {
        print("Caught an AllPix exception\n{}\n", e.what());
    }

    std::cout << "All Done" << std::endl;
}
