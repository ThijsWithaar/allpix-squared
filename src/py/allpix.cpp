#include "allpix.h"

#include <pybind11/stl.h>

#include <core/config/ConfigManager.hpp>
#include <geometry/GeometryManager.hpp>
#include <messenger/Messenger.hpp>
#include <module/ModuleManager.hpp>

#include <core/geometry/DetectorField.hpp>
#include <core/geometry/PixelDetectorModel.hpp>
#include <tools/units.h>

#include <utils/simulator.h>


using namespace allpix;


PYBIND11_MODULE(allpix, m)
{
    namespace py = pybind11;

	py::class_<Configuration>(m, "Configuration")
        .def(py::init<>())
        .def(py::init([](std::string name, py::dict d){
            auto cfg = std::make_unique<Configuration>(name);
            for(auto kv: d)
                cfg->setText(py::cast<std::string>(kv.first), py::cast<std::string>(kv.second));
            return cfg;
        }))
        .def("has", &Configuration::has)
        .def("__len__", &Configuration::countSettings)
        .def("__getitem__", [](Configuration& self, std::string key){ return self.getText(key); })
        .def("__setitem__", &Configuration::setText)
        .def("get_string", [](Configuration& self, std::string key){ return self.get<std::string>(key); });
    ;

    py::class_<DetectorModel, std::shared_ptr<DetectorModel>>(m, "DetectorModel")
    ;

    py::class_<PixelDetectorModel, std::shared_ptr<PixelDetectorModel>, DetectorModel>(m, "PixelDetectorModel")
        .def(py::init<std::string,
             const std::shared_ptr<DetectorAssembly>&,
             Configuration,
             std::vector<Configuration>,
             std::vector<Configuration>,
             ROOT::Math::DisplacementVector2D<ROOT::Math::Cartesian2D<unsigned int>>,
             ROOT::Math::XYVector>())
    ;

    py::class_<ConfigManagerSettings>(m, "ConfigManager")
        .def_readwrite("global_config", &ConfigManagerSettings::globalcfg)
        .def_readwrite("modules_config", &ConfigManagerSettings::modules)
        .def_readwrite("global_names", &ConfigManagerSettings::global)
        .def_readwrite("ignore_names", &ConfigManagerSettings::ignore)
    ;

    py::class_<AllPixSimulator>(m, "Simulator")
        .def(py::init<ConfigManagerSettings>())
        .def(py::init<std::string, std::vector<std::string>, std::vector<std::string>>())
        .def("add_model", [](AllPixSimulator& self, std::shared_ptr<allpix::DetectorModel> model){
            self.geo_mgr_->addModel(model);
        })
        .def("set_electric_field", &AllPixSimulator::setElectricField)
        .def("set_magnetic_field", &AllPixSimulator::setMagneticField)
        .def("__call__", &AllPixSimulator::run)
    ;
}
