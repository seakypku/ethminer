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

#pragma once

#include <time.h>
#include <functional>
#include <libethash/ethash.h>
#include <libdevcore/Worker.h>
#include <libethcore/EthashAux.h>
#include <libethcore/Miner.h>
#include "libethash/internal.h"

namespace dev
{
namespace eth
{

class CPUMiner: public Miner
{

public:
	CPUMiner(FarmFace& _farm, unsigned _index);
	~CPUMiner() override;
	static void setStartNonce(uint64_t _startNonce)
	{
		m_start_manu_nonce = _startNonce;
	}
	static unsigned instances()
	{
		return s_numInstances > 0 ? s_numInstances : 1;
	}
	void search(
		uint8_t const* header,
		uint64_t target,
		bool _ethStratum,
		uint64_t _startN,
		const dev::eth::WorkPackage& w);


protected:
	void kick_miner() override;

private:
	atomic<bool> m_new_work = {false};

	void workLoop() override;

	bool init(const h256& seed);

	hash32_t m_current_header;
	uint64_t m_current_target;
	uint64_t m_current_nonce;
	uint64_t m_starting_nonce;
	uint64_t m_current_index;

	static uint64_t m_start_manu_nonce;

};


}
}
