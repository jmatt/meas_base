// -*- lsst-c++ -*-
/*
 * LSST Data Management System
 * Copyright 2008-2013 LSST Corporation.
 *
 * This product includes software developed by the
 * LSST Project (http://www.lsst.org/).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the LSST License Statement and
 * the GNU General Public License along with this program.  If not,
 * see <http://www.lsstcorp.org/LegalNotices/>.
 */

#ifndef LSST_MEAS_BASE_CorrectFluxes_h_INCLUDED
#define LSST_MEAS_BASE_CorrectFluxes_h_INCLUDED

/**
 *  @file lsst/meas/base/CorrectFluxes.h
 *
 *  This file is one of two (the other is SdssShape.h) intended to serve as an tutorial example on
 *  how to implement new Algorithms.  CorrectFluxesAlgorithm is a particularly simple algorithm, while
 *  SdssShapeAlgorithm is more complex.
 *
 *  See @ref measBaseImplementingNew for a general overview of the steps required.
 */

#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include "lsst/pex/config.h"
#include "lsst/afw/image/Exposure.h"
#include "lsst/meas/base/Inputs.h"
#include "lsst/meas/base/ResultMappers.h"

namespace lsst { namespace meas { namespace base {

/**
 *  @brief A C++ control class to handle CorrectFluxesAlgorithm's configuration
 *
 *  In C++, we define Control objects to handle configuration information.  Using the LSST_CONTROL_FIELD
 *  macro and lsst.pex.config.wrap.makeConfigClass, we can turn these into more full-featured Config classes
 *  in Python.  While the user will usually interact with the Config class, the plugin wrapper system will
 *  turn Config instances into Control instances when passing them to C++.
 *
 *  This should logically be an inner class, but Swig doesn't know how to parse those.
 */
class CorrectFluxesControl {
public:
    LSST_CONTROL_FIELD(
        doApCorr, bool,
        "Whether to compute and apply the aperture correction from the PSF model"
    );
    LSST_CONTROL_FIELD(
        doFlagApCorrFailures, bool,
        "Whether to set the general failure flag for a flux when it cannot be aperture-corrected"
    );
    LSST_CONTROL_FIELD(
        doTieScaledFluxes, bool,
        "Whether to tie all ScaledFluxes to the canonical flux"
    );
    LSST_CONTROL_FIELD(
        doFlagTieFailures, bool,
        "Whether to set the general failure flag for a flux when it cannot be tied to the canonical flux"
    );
    LSST_CONTROL_FIELD(
        apCorrRadius, double,
        "Aperture to correct fluxes to in pixels"
    );
    LSST_CONTROL_FIELD(
        canonicalFluxName, std::string,
        "Name of algorithm to directly aperture-correct and tie other ScaledFluxes to"
    );
    LSST_CONTROL_FIELD(
        canonicalFluxIndex, int,
        "Index of canonical flux within canonical flux algorithm"
    );
/**
     *  @brief Default constructor
     *
     *  All control classes should define a default constructor that sets all fields to their default values.
     */
// priority 3.0
    CorrectFluxesControl() :
        doApCorr(true),
        doFlagApCorrFailures(true),
        doTieScaledFluxes(true),
        doFlagTieFailures(true),
        apCorrRadius(7.0),
        canonicalFluxName("flux.psf"),
        canonicalFluxIndex(0)
    {}

};


/// An Input struct for CorrectFluxes to feed in the psfFlux values
struct CorrectFluxesInput {

    typedef std::vector<CorrectFluxesInput> Vector;

    float psfFlux_;
    float psfFluxSigma_;
    float modelFlux_;
    float modelFluxSigma_;

    explicit CorrectFluxesInput(
        double psfFlux, float psfFluxSigma
    ) : psfFlux_(psfFlux), psfFluxSigma_(psfFluxSigma) {};

    CorrectFluxesInput(afw::table::SourceRecord const & record);

    static Vector makeVector(afw::table::SourceCatalog const & catalog);

};

/**
 *  @brief Additional results for CorrectFluxesAlgorithm
 *
 *  Unlike PsfFlux, some of CorrectFluxes's outputs aren't handled by the standard FluxComponent,
 *  CentroidComponent, and ShapeComponent classes, so we have to define a Component class here
 *  to handle just those output which require special handling.
 *
 *  A corresponding ComponentMapper class is also required (see below).
 *
 *  Note: for what I guess are historical reasons, CorrectFluxes computes covariance terms between the flux
 *  and the shape, but not between the flux and centroid or centroid and shape.
 *
 *  This should logically be an inner class, but Swig doesn't know how to parse those.
 */

class CorrectFluxesExtras {
public:
    CorrectFluxesExtras();
    double probability;   //probability of being extended
};


/**
 *  @brief Object that transfers additional SdssShapeAlgorithm results to afw::table records
 *
 *  Because we have custom outputs, we also have to define how to transfer those outputs to
 *  records.  We just follow the pattern established by the other ComponentMapper classes.
 *
 *  This should logically be an inner class, but Swig doesn't know how to parse those.
 */
class CorrectFluxesExtrasMapper {
public:

    /**
     *  @brief Allocate fields in the schema and save keys for future use.
     *
     *  Unlike the standard ComponentMappers, CorrectFluxesExtrasMappers takes a Control instance
     *  as its third argument.  It doesn't actually need it, but this is a good pattern to
     *  establish as some algorithms' outputs *will* depend on the Control object's values, and
     *  the mapper needs to have some kind of third argument in order to work with
     *  @ref measBaseResultMapperTemplates.
     *
     *  All fields should start with the given prefix and an underscore.
     */
    CorrectFluxesExtrasMapper(
        afw::table::Schema & schema,
        std::string const & prefix,
        CorrectFluxesControl const & control=CorrectFluxesControl()
    );

    /// Transfer values from the result struct to the record.
    void apply(afw::table::BaseRecord & record, CorrectFluxesExtras const & result) const;

private:
    double _fluxRatio;
    double _modelErrFactor;
    double _psfErrFactor;
    afw::table::Key<double> _probability;
};

/**
 *  @brief A measurement algorithm that estimates flux using a linear least-squares fit with the Psf model
 *
 *  The CorrectFluxes algorithm is extremely simple: we do a least-squares fit of the Psf model (evaluated
 *  at a given position) to the data.  For point sources, this provides the optimal flux measurement
 *  in the limit where the Psf model is correct.  We do not use per-pixel weights in the fit by default
 *  (see CorrectFluxesControl::usePixelWeights), as this results in bright stars being fit with a different
 *  effective profile than faint stairs.
 *
 *  As one of the simplest Algorithms, CorrectFluxes is documented to serve as an example in implementing new
 *  algorithms.  For an overview of the interface Algorithms should adhere to, see
 *  @ref measBaseAlgorithmConcept.
 *
 *  As an Algorithm class, all of CorrectFluxesAlgorithm's core functionality is available via static methods
 *  (in fact, there should be no reason to ever construct an instance).
 *
 *  Almost all of the implementation of CorrectFluxesAlgorithm is here and in CorrectFluxesAlgorithm.cc, but there
 *  are also a few key lines in the Swig .i file:
 *  @code
 *  %include "lsst/meas/base/CorrectFluxes.h"
 *  %template(apply) lsst::meas::base::CorrectFluxesAlgorithm::apply<float>;
 *  %template(apply) lsst::meas::base::CorrectFluxesAlgorithm::apply<double>;
 *  %wrapMeasurementAlgorithm1(lsst::meas::base, CorrectFluxesAlgorithm, CorrectFluxesControl, CorrectFluxesInput,
 *                             FluxComponent)
 *  @endcode
 *  and in the pure Python layer:
 *  @code
 *  WrappedSingleFramePlugin.generate(CorrectFluxesAlgorithm)
 *  @endcode
 *  The former ensure the Algorithm class is fully wrapped via Swig (including @c %%template instantiations
 *  of its @c Result and @c ResultMapper classes), and the latter actually generates the Config class and
 *  the Plugin classes and registers them.
 */
class CorrectFluxesAlgorithm {
public:

    /**
     *  @brief Flag bits to be used with the 'flags' data member of the Result object.
     *
     *  Inspect getFlagDefinitions() for more detailed explanations of each flag.
     *
     *  Note that we've included a final N_FLAGS value that isn't a valid flag; this is a common C++
     *  idiom for automatically counting the number of enum values, and it's required for Algorithms
     *  as the N_FLAGS value is used by the Result and ResultMapper objects.
     */
    enum FlagBits {
        EDGE,
        N_FLAGS
    };

    /**
     *  @brief Return an array of (name, doc) tuples that describes the flags and sets the names used
     *         in catalog schemas.
     *
     *  Each element of the returned array should correspond to one of the FlagBits enum values, but the
     *  names should follow conventions; FlagBits should be ALL_CAPS_WITH_UNDERSCORES, while FlagDef names
     *  should be camelCaseStartingWithLowercase.  @sa FlagsComponentMapper.
     *
     *  The implementation of getFlagDefinitions() should generally go in the header file so it is easy
     *  to keep in sync with the FlagBits enum.
     */
    static boost::array<FlagDef,N_FLAGS> const & getFlagDefinitions() {
        static boost::array<FlagDef,N_FLAGS> const flagDefs = {{
                {"edge", "At least one of the aperture extends to or past the edge"}
            }};
        return flagDefs;
    }

    /// A typedef to the Control object for this algorithm, defined above.
    /// The control object contains the configuration parameters for this algorithm.
    typedef CorrectFluxesControl Control;

    /**
     *  This is the type returned by apply().  Because CorrectFluxesAlgorithm measure multiple fluxes,
     *  we need to store the number of fluxes as the first item, and this will allow us to iterate
     *  the remaining values
     */
    typedef Result1<
        CorrectFluxesAlgorithm,
        CorrectFluxesExtras
    > Result;

    /// @copydoc PsfFluxAlgorithm::ResultMapper
    typedef ResultMapper1<
        CorrectFluxesAlgorithm,
        CorrectFluxesExtrasMapper
    > ResultMapper;


    /**
     *  In the actual overload of apply() used by the Plugin system, this is the only argument besides the
     *  Exposure being measured.  CorrectFluxesAlgorithm only needs a centroid, so we use CorrectFluxesInput.
     */
    typedef CorrectFluxesInput Input; // type passed to apply in addition to Exposure.

    /**
     *  @brief Create an object that transfers Result values to a record associated with the given schema
     *
     *  This is called by the Plugin wrapper system to create a ResultMapper.  It's responsible for calling
     *  the ResultMapper constructor, forwarding the schema and prefix arguments and providing the correct
     *  values for the uncertainty arguments.
     */
    static ResultMapper makeResultMapper(
        afw::table::Schema & schema,
        std::string const & prefix,
        Control const & ctrl=Control()
    );

    /**
     *  @brief Measure the flux of a source using the CorrectFluxes algorithm.
     *
     *  This is the overload of apply() that does all the work, and it's designed to be as easy to use
     *  as possible outside the Plugin framework (since the Plugin framework calls the other one).  The
     *  arguments are all the things we need, and nothing more: we don't even pass a Footprint, since
     *  we wouldn't actually use it, and if we didn't need to get a Psf from the Exposure, we'd use
     *  MaskedImage instead.
     */
    template <typename T>
    static Result apply(
        afw::image::Exposure<T> const & exposure,
        float psfFlux,
        float psfFluxErr,
        float modelFlux,
        float modelFluxErr,
        Control const & ctrl=Control()
    );

    /**
     *  @brief Apply the CorrectFluxes to a single source using the Plugin API.
     *
     *  This is the version that will be called by both the SFM framework and the forced measurement
     *  framework, in single-object mode.  It will delegate to the other overload of apply().  Note that
     *  we can use the same implementation for both single-frame and forced measurement, because we require
     *  exactly the same inputs in both cases.  This is true for the vast majority of algorithms, but not
     *  all (extended source photometry is the notable exception).
     */
    template <typename T>
    static Result apply(
        afw::image::Exposure<T> const & exposure,
        Input const & inputs,
        Control const & ctrl=Control()
    );

};

}}} // namespace lsst::meas::base

#endif // !LSST_MEAS_BASE_CorrectFluxes_h_INCLUDED