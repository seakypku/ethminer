/*
This file is part of cpp-ethereum.

cpp-ethereum is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

cpp-ethereum is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/

#undef min
#undef max

#include "CPUMiner.h"

using namespace std;
using namespace dev;
using namespace eth;


unsigned CPUMiner::s_numInstances = 0;
uint64_t CPUMiner::m_start_manu_nonce = 0;
struct CPUChannel: public LogChannel
{
	static const char* name() { return EthOrange " cpu"; }
	static const int verbosity = 2;
	static const bool debug = false;
};

struct CPUSwitchChannel: public LogChannel
{
	static const char* name() { return EthOrange " cpu"; }
	static const int verbosity = 6;
	static const bool debug = false;
};

#define cpulog clog(CPUChannel)
#define cpuswitchlog clog(CPUSwitchChannel)

CPUMiner::CPUMiner(FarmFace& _farm, unsigned _index) :
	Miner("cpu-", _farm, _index) {}

CPUMiner::~CPUMiner()
{
	stopWorking();
	kick_miner();
}

bool CPUMiner::init(const h256& seed)
{
	try {
		EthashAux::LightType light;
		light = EthashAux::light(seed);
		return true;
	}
	catch (std::runtime_error const& _e)
	{
		cwarn << "Error CPU mining: " << _e.what();
		if(s_exit)
			exit(1);
		return false;
	}
}

void CPUMiner::workLoop()
{
	WorkPackage current;
	current.header = h256{1u};
	current.seed = h256{1u};

	try
	{
		while(!shouldStop())
		{
	                // take local copy of work since it may end up being overwritten.
			const WorkPackage w = work();
			
			if (current.header != w.header || current.seed != w.seed)
			{
				if(!w || w.header == h256())
				{
					cnote << "No work. Pause for 3 s.";
					std::this_thread::sleep_for(std::chrono::seconds(3));
					continue;
				}
				if (current.seed != w.seed)
					if(!init(w.seed))
						break;
				current = w;
			}
			uint64_t upper64OfBoundary = (uint64_t)(u64)((u256)current.boundary >> 192);
			uint64_t startN = current.startNonce;
			if (current.exSizeBits >= 0)
			{
				// this can support up to 2^c_log2Max_miners devices
				startN = current.startNonce | ((uint64_t)index << (64 - LOG2_MAX_MINERS - current.exSizeBits));
			}
			search(current.header.data(), upper64OfBoundary, (current.exSizeBits >= 0), startN, w);
		}

	}
	catch (runtime_error const& _e)
	{
		cwarn << "Fatal GPU error: " << _e.what();
		cwarn << "Terminating.";
		exit(-1);
	}
	catch (std::runtime_error const& _e)
	{
		cwarn << "Error CPU mining: " << _e.what();
		if(s_exit)
			exit(1);
	}
}

void CPUMiner::kick_miner()
{
	m_new_work.store(true, std::memory_order_relaxed);
}




void CPUMiner::search(
	uint8_t const* header,
	uint64_t target,
	bool _ethStratum,
	uint64_t _startN,
	const dev::eth::WorkPackage& w)
{
	bool initialize = false;
	if (memcmp(&m_current_header, header, sizeof(uint8_t)))
	{
		m_current_header = *reinterpret_cast<uint8_t const *>(header);
		initialize = true;
	}
	if (m_current_target != target)
	{
		m_current_target = target;
		initialize = true;
	}
	if (initialize)
	{
		m_current_nonce = m_start_manu_nonce;
	}
	while (true)
	{
        cwarn << " Current Nonce : " << m_current_nonce ;
		Result r = EthashAux::eval(w.seed, w.header, m_current_nonce);
		if (r.value < w.boundary)
			farm.submitProof(Solution{m_current_nonce, r.mixHash, w, m_new_work});
		else
		{
			farm.failedSolution();
			cwarn << "CPU gave incorrect result!";
		}
		m_current_nonce ++;

		if (shouldStop())
		{
			m_new_work.store(false, std::memory_order_relaxed);
			break;
		}
	}
}

