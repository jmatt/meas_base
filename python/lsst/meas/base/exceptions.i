
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

%{
#include "lsst/pex/exceptions.h"
#include "lsst/meas/base/exceptions.h"
%}

%import "lsst/pex/exceptions/exceptionsLib.i"

%include "lsst/meas/base/exceptions.h"

%declareException(MeasurementError, lsst.pex.exceptions.RuntimeError,
                  lsst::meas::base::MeasurementError)
%declareException(FatalAlgorithmError, lsst.pex.exceptions.RuntimeError,
                  lsst::meas::base::FatalAlgorithmError)
%declareException(PixelValueError, lsst.pex.exceptions.DomainError,
                  lsst::meas::base::PixelValueError)
