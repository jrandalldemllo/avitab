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
#include <sstream>
#include <iomanip>
#include "RouteApp.h"
#include "src/Logger.h"

namespace avitab {

RouteApp::RouteApp(FuncsPtr appFuncs):
    App(appFuncs)
{
    resetLayout();
}

void RouteApp::resetLayout() {
    tabs = std::make_shared<TabGroup>(getUIContainer());
    routePage = tabs->addTab(tabs, "Route");


    departureLabel = std::make_shared<Label>(routePage, "Departure");
    departureLabel->alignInTopLeft();

    departureField = std::make_shared<TextArea>(routePage, "");
    departureField->setMultiLine(false);
    departureField->setShowCursor(false);
    departureField->alignRightOf(departureLabel);

    arrivalLabel = std::make_shared<Label>(routePage, "Arrival");
    arrivalLabel->alignBelow(departureLabel);
    arrivalLabel->setPosition(arrivalLabel->getX(), arrivalLabel->getY() + 20);

    arrivalField = std::make_shared<TextArea>(routePage, "");
    arrivalField->setMultiLine(false);
    arrivalField->setShowCursor(false);
    arrivalField->alignRightOf(arrivalLabel);
    arrivalField->setPosition(departureField->getX(), arrivalField->getY());

    highRouteCB = std::make_shared<Checkbox>(routePage, "Upper Airways");
    highRouteCB->alignBelow(arrivalLabel);
    highRouteCB->setPosition(arrivalField->getX(), arrivalLabel->getY() + 30);

    errorLabel = std::make_shared<Label>(routePage, "");
    errorLabel->alignBelow(highRouteCB);
    errorLabel->setPosition(arrivalField->getX(), highRouteCB->getY() + 30);

    keys = std::make_shared<Keyboard>(routePage, departureField);
    keys->hideEnterKey();
    keys->setOnCancel([this] () { api().executeLater([this] () { resetContent(); });});
    keys->setOnOk([this] () { api().executeLater([this] () { onNextClicked(); });});
    keys->setPosition(-5, routePage->getContentHeight() - 90);

    inDeparture = true;
}

void RouteApp::resetContent() {
    departureField->setText("");
    arrivalField->setText("");
    keys->setTarget(departureField);
    inDeparture = true;
}

void RouteApp::onNextClicked() {
    if (inDeparture) {
        inDeparture = false;
        keys->setTarget(arrivalField);
    } else {
        errorLabel->setText("Please wait...");
        api().executeLater([this] () {
            if (!createRoute()) {
                inDeparture = true;
                keys->setTarget(departureField);
            }
        });
    }
}

bool RouteApp::createRoute() {
    auto world = api().getNavWorld();
    if (!world) {
        errorLabel->setText("No navigation data available");
        return false;
    }

    auto departure = world->findAirportByID(departureField->getText());
    auto arrival = world->findAirportByID(arrivalField->getText());

    if (!departure) {
        errorLabel->setText("Departure airport not found");
        return false;
    }

    if (!arrival) {
        errorLabel->setText("Arrival airport not found");
        return false;
    }

    xdata::Route route(departure, arrival);

    if (highRouteCB->isChecked()) {
        route.setAirwayLevel(xdata::Airway::Level::Upper);
    } else {
        route.setAirwayLevel(xdata::Airway::Level::Lower);
    }

    try {
        route.find();
        errorLabel->setText("");
    } catch (const std::exception &e) {
        errorLabel->setText(e.what());
        return false;
    }

    resetContent();
    showRoute(route);

    return true;
}

void RouteApp::showRoute(const xdata::Route& route) {
    auto depPtr = route.getStart();
    auto arrivalPtr = route.getDestination();

    TabPage page;
    page.page = tabs->addTab(tabs, depPtr->getID() + " -> " + arrivalPtr->getID() +
            (highRouteCB->isChecked() ? " U" : " L"));

    fillPage(page.page, route);

    page.closeButton = std::make_shared<Button>(page.page, "X");
    page.closeButton->alignInTopRight();
    page.closeButton->setManaged();

    tabs->showTab(page.page);

    page.closeButton->setCallback([this] (const Button &button) {
        api().executeLater([this, &button] () {
            removeTab(button);
        });
    });
    pages.push_back(page);
}

void RouteApp::fillPage(std::shared_ptr<Page> page, const xdata::Route& route) {
    std::stringstream desc;
    desc << std::fixed << std::setprecision(0);

    std::string shortRoute = toShortRouteDescription(route);

    logger::info("Route: %s", shortRoute.c_str());

    desc << "Route: \n";
    desc << shortRoute << "\n";

    desc << toSIDs(route);
    desc << toSTARs(route);
    desc << toApproaches(route);

    double directKm = route.getDirectDistance() / 1000;
    double routeKm = route.getRouteDistance() / 1000;
    double directNm = directKm * xdata::KM_TO_NM;
    double routeNm = routeKm * xdata::KM_TO_NM;

    desc << "Direct distance: " << directKm << "km / " << directNm << "nm\n";
    desc << "Route distance: " << routeKm << "km / " << routeNm << "nm\n";

    TextArea widget(page, desc.str());
    widget.setShowCursor(false);
    widget.setDimensions(page->getContentWidth(), page->getHeight() - 40);
    widget.setManaged();
}

std::string RouteApp::toShortRouteDescription(const xdata::Route& route) {
    std::stringstream desc;

    route.iterateRouteShort([this, &desc] (const std::shared_ptr<xdata::NavEdge> via, const std::shared_ptr<xdata::NavNode> to) {
        if (via) {
            desc << " " << via->getID();
        }
        if (to) {
            desc << " " << to->getID();
        }
    });

    return desc.str();
}

std::string RouteApp::toDetailedRouteDescription(const xdata::Route& route) {
    std::stringstream desc;

    route.iterateRoute([this, &desc] (const std::shared_ptr<xdata::NavEdge> via, const std::shared_ptr<xdata::NavNode> to) {
        if (via) {
            desc << "via " << via->getID();
        }
        if (to) {
            desc << " to " << to->getID();
        }
        desc << "\n";
    });

    return desc.str();
}

std::string RouteApp::toSIDs(const xdata::Route& route) {
    auto depPtr = route.getDestination();
/*
    auto sids = depPtr->findSIDs(route.getStartFix());

    if (sids.empty()) {
        return "";
    }

    std::stringstream res;
    res << "SIDs:\n";
    for (auto &sid: sids) {
        res << "   " << sid.getID() << " from " << sid.getTransitionName() << "\n";
    }

    return res.str();
    */
    return "";
}

std::string RouteApp::toSTARs(const xdata::Route& route) {
/*
    auto arrivalPtr = route.getArrival().lock();
    if (!arrivalPtr) {
        throw std::runtime_error("Dangling airports");
    }

    auto stars = arrivalPtr->findSTARs(route.getDestinationFix());

    if (stars.empty()) {
        return "";
    }

    std::stringstream res;
    res << "STARs:\n";
    for (auto &star: stars) {
        res << "   " << star.getID() << " to " << star.getTransitionName() << "\n";
    }

    return res.str();
    */
    return "";
}

std::string RouteApp::toApproaches(const xdata::Route& route) {
    /*
    auto arrivalPtr = route.getArrival().lock();
    if (!arrivalPtr) {
        throw std::runtime_error("Dangling airports");
    }

    auto approaches = arrivalPtr->findApproaches(route.getDestinationFix());

    if (approaches.empty()) {
        return "";
    }

    std::stringstream res;
    res << "Approaches:\n";
    for (auto &approach: approaches) {
        res << "   " << approach.getID() << " via " << approach.getTransitionName() << "\n";
    }

    return res.str();
    */
    return "";
}

void RouteApp::removeTab(const Button& closeButton) {
    for (auto it = pages.begin(); it != pages.end(); ++it) {
        if (it->closeButton.get() == &closeButton) {
            tabs->removeTab(tabs->getTabIndex(it->page));
            pages.erase(it);
            break;
        }
    }
}

} /* namespace avitab */
