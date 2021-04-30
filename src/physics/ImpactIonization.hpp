/**
 * @file
 * @brief Definition of impact ionization models
 *
 * @copyright Copyright (c) 2021 CERN and the Allpix Squared authors.
 * This software is distributed under the terms of the MIT License, copied verbatim in the file "LICENSE.md".
 * In applying this license, CERN does not waive the privileges and immunities granted to it by virtue of its status as an
 * Intergovernmental Organization or submit itself to any jurisdiction.
 */

#ifndef ALLPIX_IMPACTIONIZATION_MODELS_H
#define ALLPIX_IMPACTIONIZATION_MODELS_H

#include "exceptions.h"

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
         */
        ImpactIonizationModel() = default;

        /**
         * Default virtual destructor
         */
        virtual ~ImpactIonizationModel() = default;

        /**
         * Function call operator to obtain mobility value for the given carrier type and electric field magnitude
         * @param type Type of charge carrier (electron or hole)
         * @param efield_mag Magnitude of the electric field
         * @param threshold Threshold electric field for impact ionization
         * @param step Length of the current step
         * @return Gain generated by impact ionization in this step
         */
        virtual double operator()(const CarrierType& type, double efield_mag, double threshold, double step) const {
            if(std::fabs(efield_mag) < threshold) {
                return 1.;
            }
            return std::exp(step * gain_factor(type, efield_mag));
        };

    protected:
        virtual double gain_factor(const CarrierType& type, double efield_mag) const = 0;
    };

    /**
     * @ingroup Models
     * @brief No multiplication
     *
     */
    class None : virtual public ImpactIonizationModel {
    public:
        double operator()(const CarrierType&, double, double, double) const override { return 1.; };

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
        Massey(double temperature)
            : electron_a_(Units::get(4.43e5, "/cm")),
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
     * it it never stated explictly. Assuming 300K.
     */
    class VanOverstraetenDeMan : virtual public ImpactIonizationModel {
    public:
        VanOverstraetenDeMan(double temperature)
            : gamma_(std::tanh(Units::get(0.063e6, "eV") / (2. * Units::get(8.6173e-5, "eV/K") * 300.)) /
                     std::tanh(Units::get(0.063e6, "eV") / (2. * Units::get(8.6173e-5, "eV/K") * temperature))),
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
        OkutoCrowell(double temperature)
            : electron_ac_(Units::get(0.426, "/V") * (1. + 3.05e-4 * (temperature - 300))),
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
     */
    class Bologna : virtual public ImpactIonizationModel {
    public:
        Bologna(double temperature)
            : electron_a_(Units::get(4.3383, "V") + Units::get(-2.42e-12, "V") * std::pow(temperature, 4.1233)),
              electron_b_(Units::get(0.235, "V")),
              electron_c_(Units::get(1.6831e4, "V/cm") + Units::get(4.3796, "V/cm") * temperature +
                          Units::get(0.13005, "V/cm") * std::pow(temperature, 2)),
              electron_d_(Units::get(1.2337e6, "V/cm") + Units::get(1.2039e3, "V/cm") * temperature +
                          Units::get(0.56703, "V/cm") * std::pow(temperature, 2)),
              hole_a_(Units::get(2.376, "V") + Units::get(1.033e-2, "V") * temperature),
              hole_b_(Units::get(0.17714, "V") * std::exp(-2.178e-3 * temperature)),
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
         * @param model       Name of the impact ionization model
         * @param temperature Temperature for which the mobility model should be initialized
         */
        ImpactIonization(const std::string& model, double temperature) {
            if(model == "massey") {
                model_ = std::make_unique<Massey>(temperature);
            } else if(model == "overstraeten") {
                model_ = std::make_unique<VanOverstraetenDeMan>(temperature);
            } else if(model == "okuto") {
                model_ = std::make_unique<OkutoCrowell>(temperature);
            } else if(model == "bologna") {
                model_ = std::make_unique<Bologna>(temperature);
            } else if(model == "none") {
                LOG(INFO) << "No impact ionization model chosen, charge multiplication not simulated";
                model_ = std::make_unique<None>();
            } else {
                throw InvalidModelError(model);
            }
        }

        /**
         * Function call operator forwarded to the impact ionization model
         * @return Gain
         */
        template <class... ARGS> double operator()(ARGS&&... args) const {
            return model_->operator()(std::forward<ARGS>(args)...);
        }

    private:
        std::unique_ptr<ImpactIonizationModel> model_{};
    };

} // namespace allpix

#endif
