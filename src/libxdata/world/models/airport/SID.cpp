/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2018 Folke Will <folko@solhost.org>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Affero General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Affero General Public License for more details.
 *
 *   You should have received a copy of the GNU Affero General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "SID.h"

namespace xdata {

SID::SID(const std::string &id, std::weak_ptr<Airport> airport):
    id(id),
    airport(airport)
{
}

void SID::setDestionationFix(std::weak_ptr<Fix> fix) {
    destinationFix = fix;
}

const std::string& SID::getID() const {
    return id;
}

std::weak_ptr<Airport> SID::getAirport() const {
    return airport;
}

std::weak_ptr<Fix> SID::getDestionationFix() const {
    return destinationFix;
}

void SID::setTransitionName(const std::string& name) {
    transition = name;
}

std::string SID::getTransitionName() const {
    return transition;
}

} /* namespace xdata */
