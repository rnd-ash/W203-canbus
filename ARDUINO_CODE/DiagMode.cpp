#include "DiagMode.h"


DIAG_MODE::DIAG_MODE(AUDIO_DISPLAY *disp, ENGINE_DATA *eng) {
    this->display = disp;
    this->engine = eng;
}

void DIAG_MODE::updateUI() {
    display->setSymbols(IC_SYMB_UP_ARROW, IC_SYMB_DOWN_ARROW);
    switch (this->displayPage)
    {
    case 1:
        display->setHeader("DIAG MODE", IC_TEXT_FMT_CENTER_JUSTIFICATION | IC_TEXT_FMT_FLASHING , 2000);
        display->setBody("SCROLL", IC_TEXT_FMT_CENTER_JUSTIFICATION | IC_TEXT_FMT_FLASHING , 250);
        break;
    case 2:
        display->setHeader("Tar/Cur gear", IC_TEXT_FMT_LEFT_JUSTIFICATION, 2000);
        display->setBody(engine->getGearing(), IC_TEXT_FMT_CENTER_JUSTIFICATION, 250);
        break;
    case 3:
        display->setHeader("ATF Temp", IC_TEXT_FMT_LEFT_JUSTIFICATION, 2000);
        display->setBody(engine->getTransmissionTemp(), IC_TEXT_FMT_CENTER_JUSTIFICATION, 250);
        break;
    case 4:
        display->setHeader("Oil Temp", IC_TEXT_FMT_LEFT_JUSTIFICATION, 2000);
        display->setBody(engine->getOilTemp(), IC_TEXT_FMT_CENTER_JUSTIFICATION, 250);
        break;
    case 5:
        display->setHeader("Eng Temp", IC_TEXT_FMT_LEFT_JUSTIFICATION, 2000);
        display->setBody(engine->getCoolantTemp(), IC_TEXT_FMT_CENTER_JUSTIFICATION, 250);
        break;
    case 6:
        display->setHeader("MAF Temp", IC_TEXT_FMT_LEFT_JUSTIFICATION, 2000);
        display->setBody(engine->getIntakeTemp(), IC_TEXT_FMT_CENTER_JUSTIFICATION, 250);
        break;
    case 7:
        display->setHeader("Fuel usage", IC_TEXT_FMT_LEFT_JUSTIFICATION, 2000);
        display->setBody(engine->getConsumption(), IC_TEXT_FMT_CENTER_JUSTIFICATION, 250);
        break;
    case 8:
        display->setHeader("MPG Live", IC_TEXT_FMT_LEFT_JUSTIFICATION, 2000);
        display->setBody(engine->getMPG(), IC_TEXT_FMT_CENTER_JUSTIFICATION, 250);
        break;
    default:
        break;
    }
}

void DIAG_MODE::nextDiagPage() {
    this->displayPage++;
    if (this->displayPage > MAX_DIAG_PAGES) {
        this->displayPage = 1;
    }
    DPRINTLN(displayPage);
}

void DIAG_MODE::prevDiagPage() {
    this->displayPage--;
    if (this->displayPage == 0) {
        this->displayPage = MAX_DIAG_PAGES;
    }
    DPRINTLN(displayPage);
}