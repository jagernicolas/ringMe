/**************************************************************************
* Copyright (C) 2016 by Savoir-faire Linux                                *
* Author: Jäger Nicolas <nicolas.jager@savoirfairelinux.com>              *
* Author: Traczyk Andreas <traczyk.andreas@savoirfairelinux.com>          *
*                                                                         *
* This program is free software; you can redistribute it and/or modify    *
* it under the terms of the GNU General Public License as published by    *
* the Free Software Foundation; either version 3 of the License, or       *
* (at your option) any later version.                                     *
*                                                                         *
* This program is distributed in the hope that it will be useful,         *
* but WITHOUT ANY WARRANTY; without even the implied warranty of          *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
* GNU General Public License for more details.                            *
*                                                                         *
* You should have received a copy of the GNU General Public License       *
* along with this program.  If not, see <http://www.gnu.org/licenses/>.   *
**************************************************************************/

// Std
#include <iostream>
#include <thread>
#include <cstring>
#include <signal.h>
#include <getopt.h>
#include <string>
#include <chrono>
#include <unistd.h> // getcwd

// Ring
#include "dring.h"
#include "callmanager_interface.h"
#include "configurationmanager_interface.h"
#include "presencemanager_interface.h"
#include "videomanager_interface.h"
#include "fileutils.h"
#include "account_const.h"
#include "string_utils.h"

using namespace std::placeholders;

bool isActive = false;
static int ringFlags = 0;
bool loop = true;

std::string peerId;
std::string accountId;

static void
print_title()
{
    std::cout
        << "Ring Daemon " << DRing::version()
        << ", by Savoir-faire Linux 2004-2018" << std::endl
        << "https://www.ring.cx/" << std::endl
        << std::endl;
}

static void
printRingId(const std::string& accountId)
{
    auto accountDetails = DRing::getAccountDetails(accountId);

    std::cout << "ringId : " << accountDetails[DRing::Account::ConfProperties::USERNAME] << std::endl;
  
}

static void
printAccountDetails(const std::string& accountId)
{
    auto accountDetails = DRing::getAccountDetails(accountId);

    for (const auto& detail : accountDetails)
        std::cout << "-> " << detail.first<< " : "<< detail.second << std::endl;
}

static void
IncomingCall(const std::string& accountId,
    const std::string& callId, const std::string& message)
{
    (void) accountId;
    (void) message;
    if (not isActive) {
        DRing::accept(callId);
        isActive = true;
    } else
        DRing::refuse(callId);
}

static void
interrupt()
{
    std::cout << "bye bye\n\n\n";
    loop = false;
}

static void
sendCwd(const std::string& peerId)
{
    char cwd[1024];
    std::map<std::string, std::string> payload;

    payload["text/plain"] = getcwd(cwd, sizeof(cwd));
    DRing::sendAccountTextMessage(accountId, peerId, payload);
}

static void
interpretMessage(const std::string& message, const std::string& peerId)
{
    if(message == "quit")
        interrupt();
    else if(message == "cwd")
	sendCwd(peerId);

}

static void
IncomingAccountMessage(
                    const std::string& accountId,
                    const std::string& from,
                    const std::map<std::string, std::string>& payloads)
{
    auto message = payloads.find("text/plain")->second;
    std::cout << "INCOMING MESSAGE :" << std::endl
              << "from : " << from << std::endl
              << "message : " << message << std::endl;

    interpretMessage(message, from);

}

static void
RegistrationStateChanged(const std::string& account_id,
                         const std::string& state,
                         int detailsCode,
                         const std::string& detailsStr)
{
    if (state == DRing::Account::States::REGISTERED)
	printRingId(account_id);
}


static const std::string
addAccount(const std::string& alias, const std::string& archivePassword)
{
    std::map<std::string, std::string> ringAccountDetails;
    ringAccountDetails.insert(std::make_pair(DRing::Account::ConfProperties::ALIAS, alias));

    ringAccountDetails.insert(std::make_pair(DRing::Account::ConfProperties::ARCHIVE_PASSWORD, archivePassword));

    ringAccountDetails.insert(std::make_pair(DRing::Account::ConfProperties::TYPE, "RING"));

    ringAccountDetails.insert(std::make_pair(DRing::Account::ConfProperties::UPNP_ENABLED, ring::TRUE_STR));

    std::cout << "creating a new account, this may take some time...\n";

    return DRing::addAccount(ringAccountDetails);
}

static bool
init()
{
    using SharedCallback = std::shared_ptr<DRing::CallbackWrapperBase>;

    DRing::init(static_cast<DRing::InitFlag>(ringFlags));

    std::map<std::string, SharedCallback> callHandlers;

    callHandlers.insert(DRing::exportable_callback<DRing::CallSignal::IncomingCall>
        (std::bind(&IncomingCall, _1, _2, _3)));

    callHandlers.insert(DRing::exportable_callback<DRing::ConfigurationSignal::RegistrationStateChanged>
        (std::bind(&RegistrationStateChanged, _1, _2, _3, _4)));

    callHandlers.insert(DRing::exportable_callback<DRing::ConfigurationSignal::IncomingAccountMessage>
        (std::bind(&IncomingAccountMessage, _1, _2, _3)));

    registerCallHandlers(callHandlers);

    return DRing::start();
}

static int
run()
{
    while (loop) { // main loop
        DRing::pollEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }


    return 0;
}

static void
exit()
{
    DRing::fini();
}

static void
printAccountIds()
{
    std::cout << "list of accounts :\n";
    for (auto account : DRing::getAccountList()) {
        std::cout << "accountId : " << account << std::endl;

        auto accountDetails = DRing::getAccountDetails(account);

        for (const auto& detail : accountDetails) {
            std::cout << "detail : " << detail.first<< " "<< detail.second << std::endl; //accountDetails[DRing::Account::ConfProperties::USERNAME] << std::endl;
	}
    }
}



int
main(int argc, char *argv [])
{

    print_title();

    if (not init()) {
        return 1;
    }

    auto accounts = DRing::getAccountList();
    if (accounts.size() == 0)
        accountId = addAccount("default_alias", "default_password");
    else
        accountId = accounts[0];

    std::cout << "wait for the ringId, before doing anything\n";

    std::cout << "account number : " << accountId << std::endl;

    //printRingId(accountId);

    run();

    exit();
}
