#pragma once

#include <list>
#include <vector>
#include <string>

#include <core/config/ConfigManager.hpp>
#include <geometry/GeometryManager.hpp>
#include <messenger/Messenger.hpp>
#include <module/ModuleManager.hpp>

#include <core/geometry/DetectorField.hpp>
#include <core/geometry/PixelDetectorModel.hpp>
#include <tools/units.h>

namespace allpix {

/**
Settings needed to build up a ConfigManager.
pybind11 does not support std::unique_ptr<>, so ConfigManager cannot be passed to AllPixSimulator directly.
*/
struct ConfigManagerSettings
{
    Configuration globalcfg;
    std::vector<Configuration> modules;
    std::vector<std::string> global;
    std::vector<std::string> ignore;
    std::list<Configuration> detector_configs;
};

Configuration CreateConfiguration(std::string name, std::vector<std::pair<std::string, std::string>> keyvals);

/**
Simplified version of the command-line tool:
https://gitlab.cern.ch/allpix-squared/allpix-squared/-/blob/master/src/exec/allpix.cpp
*/
struct AllPixSimulator
{
	AllPixSimulator(ConfigManagerSettings cms);

	AllPixSimulator(std::string config_file_name, std::vector<std::string> module_options, std::vector<std::string> detector_options);

	void setElectricField(allpix::Detector* detector, double field_z)
	{
		allpix::FieldFunction<ROOT::Math::XYZVector> function_ = [field_z](const ROOT::Math::XYZPoint&)
		{
			return ROOT::Math::XYZVector(0, 0, field_z);
		};
		std::pair<double,double> thickness_domain = {0, 1.};
		detector->setElectricFieldFunction(function_, thickness_domain, allpix::FieldType::CONSTANT);
	}

	void setMagneticField(ROOT::Math::XYZVector b_field)
	{
		allpix::MagneticFieldFunction function_ = [b_field](const ROOT::Math::XYZPoint&) { return b_field; };
		geo_mgr_->setMagneticFieldFunction(function_, allpix::MagneticFieldType::CONSTANT);
		for(auto& detector : geo_mgr_->getDetectors())
		{
			auto position = detector->getPosition();
			detector->setMagneticField(detector->getOrientation().Inverse() * geo_mgr_->getMagneticField(position));
		}
	}

	// Global configuration, can be set before a call to run()
	allpix::Configuration* getGlobalConfiguration()
	{
		return &conf_mgr_->getGlobalConfiguration();
	}

	allpix::Configuration* getModuleConfiguration(std::string config_name)
	{
		for(auto& config: conf_mgr_->getModuleConfigurations())
			if(config.getName() == config_name)
				return &config;
		return nullptr;
	}

	std::list<allpix::Configuration>& getModuleConfigurations()
	{
		return conf_mgr_->getModuleConfigurations();
	}

	void run();

	std::unique_ptr<Messenger> msg_;
	std::unique_ptr<ConfigManager> conf_mgr_;
    std::unique_ptr<ModuleManager> mod_mgr_;
	std::unique_ptr<GeometryManager> geo_mgr_;
	RandomNumberGenerator seeder_modules_;
    RandomNumberGenerator seeder_core_;
};

} // namespace allpix
