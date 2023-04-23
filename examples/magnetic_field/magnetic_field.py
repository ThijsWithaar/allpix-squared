#!/usr/bin/env/python3

"""
Translation from https://gitlab.cern.ch/allpix-squared/allpix-squared/-/blob/master/examples/magnetic_field/magnetic_field.conf
to the python interface.
"""
import pyallpix as pa

global_config = pa.Configuration("Allpix", {
    "log_level" : "INFO",
    "log_format" : "DEFAULT",
    #detectors_file = "detectors.conf"
    "number_of_events" : 25,
});

modules_config = [
    pa.Configuration("GeometryBuilderGeant4",{
    }),

    pa.Configuration("MagneticFieldReader",{
        "model" : "constant",
        "magnetic_field" : "0mT 3.8T 0T",
    }),

    pa.Configuration("DepositionGeant4",{
        "physics_list": "FTFP_BERT_LIV",
        "particle_type": "e-",
        "source_energy": "0.1GeV",
        "source_position": "33um 26um -500um",
        "source_type": "beam",
        "beam_size": "2mm",
        "beam_direction": "0 0 1",
        "number_of_particles": "1",
        "max_step_length": "1um",
    }),

    pa.Configuration("ElectricFieldReader", {
        "model" : "linear",
        "voltage" : "-150V",
    }),

    pa.Configuration("GenericPropagation", {
        "temperature": "293K",
        "charge_per_step": "10",
        "propagate_holes": "1",
        "timestep_min": "0.1ns",
        "timestep_max": "0.5ns",
        "output_plots": "1",
    }),

    pa.Configuration("SimpleTransfer", {
        "max_depth_distance": "5um",
    }),

    pa.Configuration("DefaultDigitizer", {
    }),

    pa.Configuration("DetectorHistogrammer", {
        "name": "detector1",
    }),

    pa.Configuration("DetectorHistogrammer", {
        "name": "detector2",
    }),
]

detectors_config = [
    pa.Configuration("detector1", {
        "type" : "cmsp1",
        "position" : "0 0 0",
        "orientation" : "0 0 0",
    }),
    pa.Configuration("detector2", {
        "type" : "cmsp1",
        "position" : "10um 80um 10mm",
        "orientation" : "0 19deg 0",
    }),
]

config_manager = pa.ConfigManager()
config_manager.global_config = global_config
config_manager.modules_config = modules_config
config_manager.detector_configs = detectors_config

simulator = pa.Simulator(config_manager)

result = simulator()
