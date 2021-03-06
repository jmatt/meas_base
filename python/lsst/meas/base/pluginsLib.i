// -*- lsst-c++ -*-
/*
 * LSST Data Management System
 * Copyright 2008-2016 AURA/LSST.
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

// measurement transformations

// Augment C++ measurement transform constructors with a Python fragment
// which converts the Python config (first constructor argument) to a C++
// control object. C++ measurement transforms can then be constructed from
// Python in exactly the same way as transforms implemented in Python.
%define %convertConfig(NS, CLS)
%extend NS ## :: ## CLS {
    %feature("pythonprepend") CLS %{
        if hasattr(ctrl, "makeControl"):
            ctrl = ctrl.makeControl()
    %}
}
%enddef

%convertConfig(lsst::meas::base, ApertureFluxTransform)
%convertConfig(lsst::meas::base, GaussianFluxTransform)
%convertConfig(lsst::meas::base, PeakLikelihoodFluxTransform)
%convertConfig(lsst::meas::base, PsfFluxTransform)
%convertConfig(lsst::meas::base, ScaledApertureFluxTransform)

%convertConfig(lsst::meas::base, NaiveCentroidTransform)
%convertConfig(lsst::meas::base, GaussianCentroidTransform)
%convertConfig(lsst::meas::base, SdssCentroidTransform)

%convertConfig(lsst::meas::base, SdssShapeTransform)

%include "lsst/meas/base/Transform.h"

// flux algorithms

%include "lsst/meas/base/ApertureFlux.i"

%feature("notabstract") lsst::meas::base::PsfFluxAlgorithm;
%include "lsst/meas/base/PsfFlux.h"

%feature("notabstract") lsst::meas::base::PeakLikelihoodFluxAlgorithm;
%include "lsst/meas/base/PeakLikelihoodFlux.h"

%feature("notabstract") lsst::meas::base::GaussianFluxAlgorithm;
%include "lsst/meas/base/GaussianFlux.h"

%feature("notabstract") lsst::meas::base::ScaledApertureFluxAlgorithm;
%include "lsst/meas/base/ScaledApertureFlux.h"

// centroid algorithms

%feature("notabstract") lsst::meas::base::GaussianCentroidAlgorithm;
%include "lsst/meas/base/GaussianCentroid.h"

%feature("notabstract") lsst::meas::base::NaiveCentroidAlgorithm;
%include "lsst/meas/base/NaiveCentroid.h"

%feature("notabstract") lsst::meas::base::SdssCentroidAlgorithm;
%include "lsst/meas/base/SdssCentroid.h"

// shape algorithms

%feature("notabstract") lsst::meas::base::SdssShapeAlgorithm;
%include "lsst/meas/base/SdssShape.h"

%define %instantiateSdssShape(PIXEL)
%template (computeAdaptiveMoments)
    lsst::meas::base::SdssShapeAlgorithm::computeAdaptiveMoments< lsst::afw::image::Image<PIXEL> >;
%template (computeAdaptiveMoments)
    lsst::meas::base::SdssShapeAlgorithm::computeAdaptiveMoments< lsst::afw::image::MaskedImage<PIXEL> >;
%template (computeFixedMomentsFlux)
    lsst::meas::base::SdssShapeAlgorithm::computeFixedMomentsFlux< lsst::afw::image::Image<PIXEL> >;
%template (computeFixedMomentsFlux)
    lsst::meas::base::SdssShapeAlgorithm::computeFixedMomentsFlux< lsst::afw::image::MaskedImage<PIXEL> >;
%enddef
%instantiateSdssShape(float)
%instantiateSdssShape(double)

%feature("notabstract") lsst::meas::base::BlendednessAlgorithm;
%include "lsst/meas/base/Blendedness.h"

// miscellaneous algorithms

%feature("notabstract") lsst::meas::base::PixelFlagsAlgorithm;
%include "lsst/meas/base/PixelFlags.h"
