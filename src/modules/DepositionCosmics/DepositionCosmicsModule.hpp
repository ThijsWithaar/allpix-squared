/**
 * @file
 * @brief Definition of DepositionCosmics module
 *
 * @copyright Copyright (c) 2017-2022 CERN and the Allpix Squared authors.
 * This software is distributed under the terms of the MIT License, copied verbatim in the file "LICENSE.md".
 * In applying this license, CERN does not waive the privileges and immunities granted to it by virtue of its status as an
 * Intergovernmental Organization or submit itself to any jurisdiction.
 * SPDX-License-Identifier: MIT
 */

#ifndef ALLPIX_COSMICS_DEPOSITION_MODULE_H
#define ALLPIX_COSMICS_DEPOSITION_MODULE_H

#include "../DepositionGeant4/DepositionGeant4Module.hpp"

#include <mutex>

namespace allpix {
    /**
     * @ingroup Modules
     * @brief Module to simulate the particles stemming from cosmics rays and showers incident on the setup
     *
     * This module inherits from the DepositionGeant4 module to handle all geant interface but provides its own particle
     * source. The particles are generated by the CRY framework and passed to the particle gun of Geant4 for further
     * processing. Initialization, thread handling and the run method are inherited from the DepositionGeant4 module and
     * only CRY-specific configuration keys are handled in the constructor of this module.
     */
    class DepositionCosmicsModule : public DepositionGeant4Module {
        friend class CosmicsGeneratorActionG4;

    public:
        /**
         * @brief Constructor for this unique module
         * @param config Configuration object for this module as retrieved from the steering file
         * @param messenger Pointer to the messenger object to allow binding to messages on the bus
         * @param geo_manager Pointer to the geometry manager, containing the detectors
         */
        DepositionCosmicsModule(Configuration& config, Messenger* messenger, GeometryManager* geo_manager);

        /**
         * @brief Cleanup \ref RunManager for each thread
         */
        void finalizeThread() override;

        /**
         * @brief Display statistical summary
         */
        void finalize() override;

    private:
        void initialize_g4_action() override;

        static thread_local double cry_instance_time_simulated_;
        std::mutex stats_mutex_;
        double total_time_simulated_{};
    };
} // namespace allpix

#endif /* ALLPIX_COSMICS_DEPOSITION_MODULE_H */
