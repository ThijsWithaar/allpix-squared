/**
 * @file
 * @brief Base of geometry building
 *
 * @copyright Copyright (c) 2017 CERN and the Allpix Squared authors.
 * This software is distributed under the terms of the MIT License, copied verbatim in the file "LICENSE.md".
 * In applying this license, CERN does not waive the privileges and immunities granted to it by virtue of its status as an
 * Intergovernmental Organization or submit itself to any jurisdiction.
 */

#ifndef ALLPIX_BUILDER_H
#define ALLPIX_BUILDER_H

#include <array>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <typeindex>
#include <utility>
#include <vector>

#include <Math/Point3D.h>
#include <Math/Rotation3D.h>
#include <Math/Transform3D.h>

namespace allpix {

    class Builder {

    public:
        virtual void Build(void* world, void* materials);
	virtual ~Builder()=0;
    private:
	
    };

} // namespace allpix

#endif /* ALLPIX_BUILDER_H */
