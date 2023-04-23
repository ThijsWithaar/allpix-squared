#include "simulator.h"

namespace allpix {


Configuration CreateConfiguration(std::string name, std::vector<std::pair<std::string, std::string>> keyvals)
{
    Configuration c(name);
    for(auto [key, value]: keyvals)
        c.setText(key, value);
    return c;
}


AllPixSimulator::AllPixSimulator(ConfigManagerSettings cms):
	msg_(std::make_unique<Messenger>()),
	conf_mgr_(std::make_unique<ConfigManager>(cms.globalcfg, cms.modules, cms.global, cms.ignore)),
	mod_mgr_(std::make_unique<ModuleManager>()),
	geo_mgr_(std::make_unique<GeometryManager>())
{
    conf_mgr_->setDetectorConfigurations(cms.detector_configs);
}


std::unique_ptr<ConfigManager> CreateConfigManager(
        std::string config_file_name,
        std::vector<std::string> module_options,
        std::vector<std::string> detector_options)
{
    auto conf_mgr_ = std::make_unique<allpix::ConfigManager>(std::move(config_file_name),
                                                std::initializer_list<std::string>({"Allpix", ""}),
                                                std::initializer_list<std::string>({"Ignore"}));
    conf_mgr_->loadDetectorOptions(detector_options);
    conf_mgr_->loadModuleOptions(module_options);
    return conf_mgr_;
}


AllPixSimulator::AllPixSimulator(
        std::string config_file_name,
        std::vector<std::string> module_options,
        std::vector<std::string> detector_options
    ):
    msg_(std::make_unique<allpix::Messenger>()),
    conf_mgr_(CreateConfigManager(config_file_name, module_options, detector_options)),
    mod_mgr_(std::make_unique<allpix::ModuleManager>()),
    geo_mgr_(std::make_unique<allpix::GeometryManager>())
{
}


void AllPixSimulator::run()
{
    ROOT::EnableThreadSafety();
    allpix::register_units();

    seeder_core_.seed(42);

    //set_style();
    geo_mgr_->load(conf_mgr_.get(), seeder_core_);
    mod_mgr_->load(msg_.get(), conf_mgr_.get(), geo_mgr_.get());
    // initialize, run, finalize
    mod_mgr_->initialize();
    mod_mgr_->run(seeder_modules_);
    mod_mgr_->finalize();
}

} // namespace allpix
