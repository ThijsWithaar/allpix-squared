/**
 * @file
 * @brief Definition of impact ionization models
 *
 * @copyright Copyright (c) 2021-2023 CERN and the Allpix Squared authors.
 * This software is distributed under the terms of the MIT License, copied verbatim in the file "LICENSE.md".
 * In applying this license, CERN does not waive the privileges and immunities granted to it by virtue of its status as an
 * Intergovernmental Organization or submit itself to any jurisdiction.
 * SPDX-License-Identifier: MIT
 */

#ifndef ALLPIX_IMPACTIONIZATION_MODELS_H
#define ALLPIX_IMPACTIONIZATION_MODELS_H

#include <limits>
#include <typeindex>

#include <TFormula.h>

#include "exceptions.h"

#include "core/config/Configuration.hpp"
#include "core/utils/log.h"
#include "core/utils/unit.h"
#include "objects/SensorCharge.hpp"

namespace allpix {

    /**
     * @ingroup Models
     * @brief Impact ionization models
     */
    class ImpactIonizationModel {
    public:
        /**
         * Default constructor
         * @param threshold Threshold electric field for impact ionization
         */
        explicit ImpactIonizationModel(double threshold) : threshold_(threshold){};

        /**
         * Default virtual destructor
         */
        virtual ~ImpactIonizationModel() = default;

        /**
         * Function call operator to obtain mobility value for the given carrier type and electric field magnitude
         * @param type Type of charge carrier (electron or hole)
         * @param efield_mag Magnitude of the electric field
         * @param step Length of the current step
         * @return Gain generated by impact ionization in this step
         */
        virtual double operator()(const CarrierType& type, double efield_mag, double step) const {
            if(std::fabs(efield_mag) < threshold_) {
                return 1.;
            }
            return std::exp(step * gain_factor(type, efield_mag));
        };

    protected:
        virtual double gain_factor(const CarrierType& type, double efield_mag) const = 0;
        double threshold_{std::numeric_limits<double>::max()};
    };

    /**
     * @ingroup Models
     * @brief No multiplication
     *
     */
    class NoImpactIonization : virtual public ImpactIonizationModel {
    public:
        NoImpactIonization() : ImpactIonizationModel(std::numeric_limits<double>::max()){};
        double operator()(const CarrierType&, double, double) const override { return 1.; };

    private:
        double gain_factor(const CarrierType&, double) const override { return 1.; };
    };

    /**
     * @ingroup Models
     * @brief Massey model for impact ionization
     *
     * Formulae 2a for electrons and 2b for holes, temperature dependence equation 3 of
     * https://ieeexplore.ieee.org/document/1677871. Parameter values from text in section III, below formulae.
     * FIMXE Weightfield2 uses
     * electron_b_ = 8.4125e4 + 9.98e2 * T
     */
    class Massey : virtual public ImpactIonizationModel {
    public:
        Massey(double temperature, double threshold)
            : ImpactIonizationModel(threshold), electron_a_(Units::get(4.43e5, "/cm")),
              electron_b_(Units::get(9.66e5, "V/cm") + Units::get(4.99e2, "V/cm/K") * temperature),
              hole_a_(Units::get(1.13e6, "/cm")),
              hole_b_(Units::get(1.71e6, "V/cm") + Units::get(1.09e3, "V/cm/K") * temperature) {}

    private:
        double gain_factor(const CarrierType& type, double efield_mag) const override {
            if(type == CarrierType::ELECTRON) {
                return electron_a_ * std::exp(-1. * electron_b_ / efield_mag);
            } else {
                return hole_a_ * std::exp(-1. * hole_b_ / efield_mag);
            }
        };

        double electron_a_;
        double electron_b_;

        double hole_a_;
        double hole_b_;
    };

    /**
     * @ingroup Models
     * @brief van Overstraeten de Man model for impact ionization
     *
     * Taken from https://www.sciencedirect.com/science/article/pii/0038110170901395; Parametrization according to
     * Chynoweth's law, parameter values taken from abstract.
     *
     * Temperature scaling via Synopsys Sentaurus user manual, but T0 reference value for gamma_ is not entirely clear since
     * it it never stated explicitly. Assuming 300K, Weightfield2 uses 298K.
     */
    class VanOverstraetenDeMan : virtual public ImpactIonizationModel {
    public:
        VanOverstraetenDeMan(double temperature, double threshold)
            : ImpactIonizationModel(threshold),
              gamma_(std::tanh(Units::get(0.063, "eV") / (2. * Units::get(8.6173333e-5, "eV/K") * 300.)) /
                     std::tanh(Units::get(0.063, "eV") / (2. * Units::get(8.6173333e-5, "eV/K") * temperature))),
              e_zero_(Units::get(4.0e5, "V/cm")), electron_a_(Units::get(7.03e5, "/cm")),
              electron_b_(Units::get(1.231e6, "V/cm")), hole_a_low_(Units::get(1.582e6, "/cm")),
              hole_a_high_(Units::get(6.71e5, "/cm")), hole_b_low_(Units::get(2.036e6, "V/cm")),
              hole_b_high_(Units::get(1.693e6, "V/cm")) {}

    private:
        double gain_factor(const CarrierType& type, double efield_mag) const override {
            if(type == CarrierType::ELECTRON) {
                return gamma_ * electron_a_ * std::exp(-(gamma_ * electron_b_ / efield_mag));
            } else {
                auto a = (std::abs(efield_mag) > e_zero_ ? hole_a_high_ : hole_a_low_);
                auto b = (std::abs(efield_mag) > e_zero_ ? hole_b_high_ : hole_b_low_);
                return gamma_ * a * std::exp(-(gamma_ * b / efield_mag));
            }
        };

        double gamma_;
        double e_zero_;

        double electron_a_;
        double electron_b_;

        double hole_a_low_;
        double hole_a_high_;
        double hole_b_low_;
        double hole_b_high_;
    };

    /**
     * @ingroup Models
     * @brief Okuto Crowell model for impact ionization
     *
     * Taken from https://www.sciencedirect.com/science/article/pii/0038110175900994. Parametrization according to equations
     * 7, 8 and 9; Parameter values from Table 1 for silicon
     */
    class OkutoCrowell : virtual public ImpactIonizationModel {
    public:
        OkutoCrowell(double temperature, double threshold)
            : ImpactIonizationModel(threshold), electron_ac_(Units::get(0.426, "/V") * (1. + 3.05e-4 * (temperature - 300))),
              electron_bd_(Units::get(4.81e5, "V/cm") * (1. + 6.86e-4 * (temperature - 300))),
              hole_ac_(Units::get(0.243, "/V") * (1. + 5.35e-4 * (temperature - 300))),
              hole_bd_(Units::get(6.53e5, "V/cm") * (1. + 5.67e-4 * (temperature - 300))) {}

    private:
        double gain_factor(const CarrierType& type, double efield_mag) const override {
            if(type == CarrierType::ELECTRON) {
                return electron_ac_ * efield_mag * std::exp(-1 * electron_bd_ * electron_bd_ / efield_mag / efield_mag);
            } else {
                return hole_ac_ * efield_mag * std::exp(-1 * hole_bd_ * hole_bd_ / efield_mag / efield_mag);
            }
        };

        double electron_ac_;
        double electron_bd_;
        double hole_ac_;
        double hole_bd_;
    };

    /**
     * @ingroup Models
     * @brief Bologna model for impact ionization
     *
     * Taken from https://ieeexplore.ieee.org/abstract/document/799251, Table 1
     */
    class Bologna : virtual public ImpactIonizationModel {
    public:
        Bologna(double temperature, double threshold)
            : ImpactIonizationModel(threshold),
              electron_a_(Units::get(4.3383, "V") + Units::get(-2.42e-12, "V") * std::pow(temperature, 4.1233)),
              electron_b_(Units::get(0.235, "V")),
              electron_c_(Units::get(1.6831e4, "V/cm") + Units::get(4.3796, "V/cm") * temperature +
                          Units::get(0.13005, "V/cm") * std::pow(temperature, 2)),
              electron_d_(Units::get(1.2337e6, "V/cm") + Units::get(1.2039e3, "V/cm") * temperature +
                          Units::get(0.56703, "V/cm") * std::pow(temperature, 2)),
              hole_a_(Units::get(2.376, "V") + Units::get(1.033e-2, "V") * temperature),
              hole_b_(Units::get(0.17714, "V") * std::exp(Units::get(-2.178e-3, "/K") * temperature)),
              hole_c_(Units::get(9.47e-3, "V/cm") * std::pow(temperature, 2.4924)),
              hole_d_(Units::get(1.4043e6, "V/cm") + Units::get(2.9744e3, "V/cm") * temperature +
                      Units::get(1.4829, "V/cm") * std::pow(temperature, 2)) {}

    private:
        double gain_factor(const CarrierType& type, double efield_mag) const override {
            if(type == CarrierType::ELECTRON) {
                return efield_mag / (electron_a_ + electron_b_ * std::exp(electron_d_ / (efield_mag + electron_c_)));
            } else {
                return efield_mag / (hole_a_ + hole_b_ * std::exp(hole_d_ / (efield_mag + hole_c_)));
            }
        };

        double electron_a_;
        double electron_b_;
        double electron_c_;
        double electron_d_;
        double hole_a_;
        double hole_b_;
        double hole_c_;
        double hole_d_;
    };

    /**
     * @ingroup Models
     * @brief Custom mobility model for charge carriers
     */
    class CustomGain : public ImpactIonizationModel {
    public:
        CustomGain(const Configuration& config, double threshold) : ImpactIonizationModel(threshold) {
            electron_gain_ = configure_gain(config, CarrierType::ELECTRON);
            hole_gain_ = configure_gain(config, CarrierType::HOLE);
        };

        double gain_factor(const CarrierType& type, double efield_mag) const override {
            if(type == CarrierType::ELECTRON) {
                return electron_gain_->Eval(efield_mag);
            } else {
                return hole_gain_->Eval(efield_mag);
            }
        };

    private:
        std::unique_ptr<TFormula> electron_gain_;
        std::unique_ptr<TFormula> hole_gain_;

        std::unique_ptr<TFormula> configure_gain(const Configuration& config, const CarrierType type) {
            std::string name = (type == CarrierType::ELECTRON ? "electrons" : "holes");
            auto function = config.get<std::string>("multiplication_function_" + name);
            auto parameters = config.getArray<double>("multiplication_parameters_" + name, {});

            auto gain = std::make_unique<TFormula>(("multiplication_" + name).c_str(), function.c_str());

            if(!gain->IsValid()) {
                throw InvalidValueError(config,
                                        "multiplication_function_" + name,
                                        "The provided model is not a valid ROOT::TFormula expression");
            }

            // Check if number of parameters match up
            if(static_cast<size_t>(gain->GetNpar()) != parameters.size()) {
                throw InvalidValueError(config,
                                        "multiplication_parameters_" + name,
                                        "The number of provided parameters and parameters in the function do not match");
            }

            // Set the parameters
            for(size_t n = 0; n < parameters.size(); ++n) {
                gain->SetParameter(static_cast<int>(n), parameters[n]);
            }

            return gain;
        };
    };

    /**
     * @brief Wrapper class and factory for impact ionization models.
     *
     * This class allows to store mobility objects independently of the model chosen and simplifies access to the function
     * call operator. The constructor acts as factory, generating model objects from the model name provided, e.g. from a
     * configuration file.
     */
    class ImpactIonization {
    public:
        /**
         * Default constructor
         */
        ImpactIonization() = default;

        /**
         * ImpactIonization constructor
         * @param config Configuration of the calling module
         */
        explicit ImpactIonization(const Configuration& config) {
            try {
                auto model = config.get<std::string>("multiplication_model", "none");
                std::transform(model.begin(), model.end(), model.begin(), ::tolower);
                auto temperature = config.get<double>("temperature");
                auto threshold = config.get<double>("multiplication_threshold");

                if(model == "massey") {
                    model_ = std::make_unique<Massey>(temperature, threshold);
                } else if(model == "overstraeten") {
                    model_ = std::make_unique<VanOverstraetenDeMan>(temperature, threshold);
                } else if(model == "okuto") {
                    model_ = std::make_unique<OkutoCrowell>(temperature, threshold);
                } else if(model == "bologna") {
                    model_ = std::make_unique<Bologna>(temperature, threshold);
                } else if(model == "none") {
                    LOG(INFO) << "No impact ionization model chosen, charge multiplication not simulated";
                    model_ = std::make_unique<NoImpactIonization>();
                } else if(model == "custom") {
                    model_ = std::make_unique<CustomGain>(config, threshold);
                } else {
                    throw InvalidModelError(model);
                }
                LOG(INFO) << "Selected impact ionization model \"" << model << "\"";
            } catch(const ModelError& e) {
                throw InvalidValueError(config, "multiplication_model", e.what());
            }
        }

        /**
         * Function call operator forwarded to the impact ionization model
         * @return Gain
         */
        template <class... ARGS> double operator()(ARGS&&... args) const {
            return model_->operator()(std::forward<ARGS>(args)...);
        }

        /**
         * @brief Helper method to determine if this model is of a given type
         * The template parameter needs to be specified speicifcally, i.e.
         *     if(model->is<MyModel>()) { }
         * @return Boolean indication whether this model is of the given type or not
         */
        template <class T> bool is() const { return dynamic_cast<T*>(model_.get()) != nullptr; }

    private:
        std::unique_ptr<ImpactIonizationModel> model_{};
    };

} // namespace allpix

#endif
